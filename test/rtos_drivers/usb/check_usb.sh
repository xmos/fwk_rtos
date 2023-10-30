#!/bin/bash
# Copyright 2021-2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

# help text
help()
{
   echo "Framework RTOS Drivers USB test"
   echo
   echo "Syntax: check_usb.sh [-h] adapterID_optional"
   echo
   echo "options:"
   echo "h     Print this Help."
}

# flag arguments
while getopts h option
do
    case "${option}" in
        h) help
           exit;;
    esac
done

# Get unix name for determining OS
UNAME=$(uname)

if [ "$UNAME" == "Linux" ] || [ -n "$MSYSTEM" ]; then
    TIMEOUT=timeout
elif [ "$UNAME" == "Darwin" ] ; then
    TIMEOUT=gtimeout
fi

# discern repository root
REPO_ROOT=$(git rev-parse --show-toplevel)

# assign vars
TEST_DIR=testing
TMP_FIFO=pid
TARGET_REPORT=$TEST_DIR/target.rpt
HOST_REPORT=$TEST_DIR/host.rpt
FILE_PRE_DOWNLOAD=$TEST_DIR/pre_download_data.txt
FILE_TO_DOWNLOAD=$TEST_DIR/download_data.txt
FILE_POST_DOWNLOAD=$TEST_DIR/post_download_data.txt
SERIAL_TX0_FILE="$TEST_DIR/serial_tx0_data.txt"
SERIAL_TX1_FILE="$TEST_DIR/serial_tx1_data.txt"
SERIAL_RX0_FILE="$TEST_DIR/serial_rx0_data.txt"
SERIAL_RX1_FILE="$TEST_DIR/serial_rx1_data.txt"
SERIAL_TEST_DATA_BYTES=20000
DFU_RT_VID_PID=""
DFU_MODE_VID_PID="20b1:4000"
DFU_DOWNLOAD_BYTES=1024
DFU_ALT=1
DFU_VERBOSITY="-e" # Set to "-v -v -v" libusb debug prints.
APP_STARTUP_TIME_S=10
APP_SHUTDOWN_TIME_S=5
PY_TIMEOUT_S=30
if [ ! -z "${@:$OPTIND:1}" ]
then
    ADAPTER_ID="--adapter-id ${@:$OPTIND:1}"
fi

# Max time to wait after the usb mount event has been detected. This delay adds
# basic accomodation for WSL systems. With WSL, usbipd may auto-attach the
# device which in certain cases results in a second USB mount event.
post_usb_mount_delay_s=10

function cleanup {
    rm -f "$TMP_FIFO"

    if [ $child_process_pid -gt 0 ]; then
        child_process_running=$(ps -p $child_process_pid | grep $child_process_pid)
        if [ -n "$child_process_running" ]; then
            print_and_log_test_step "Stopping child process."
            kill -INT $child_process_pid
        fi
    fi
}

function print_and_log_test_step {
    printf "\n[TEST STEP]: %s\n" "$1" >> "$HOST_REPORT"
    printf "\n[TEST STEP]: %s\n" "$1" | tee -a "$TARGET_REPORT"
}

function print_and_log_failure {
    echo "[TEST FAIL]: $1" | tee -a "$TARGET_REPORT"
    echo "[TEST FAIL]: $1" >> "$HOST_REPORT"

    echo "--------"
    echo "Host Log"
    echo "--------"

    cat "$HOST_REPORT"
    echo "- See Log: $HOST_REPORT"
    echo "- See Log: $TARGET_REPORT"
}

function generate_test_file {
    # Generate a file populated with randomized hex-string data.
    # $1 = File to generate/write to (note: append mode)
    # $2 = Number of bytes to generate

    NUM_HASH_CHARS=128 # SHA512 is 128-chars
    file_to_write=$1
    bytes_to_generate=$2

    while [ $bytes_to_generate -gt $NUM_HASH_CHARS ]; do
        x=$(echo $RANDOM | sha512sum | head -c $NUM_HASH_CHARS)
        (( bytes_to_generate -= NUM_HASH_CHARS ))
        echo -n "$x" >> "$file_to_write"
    done

    x=$(echo $RANDOM | sha512sum | head -c $bytes_to_generate)
    echo -n "$x" >> "$file_to_write"
}

function run_target {
    echo "----------"
    echo "Target Log"
    echo "----------"
    print_and_log_test_step "Starting target application."
    (xrun --xscope ${ADAPTER_ID} "$REPO_ROOT/dist/test_rtos_driver_usb.xe" 2>&1 & echo $! > $TMP_FIFO) | tee -a "$TARGET_REPORT" &
    child_process_pid=$(<$TMP_FIFO)
}

function wait_for_usb_mount {
    # Wait for application to start. By this time the application must indicate it
    # has mounted (via a log entry indicating "tud_mount_cb"); otherwise, no further
    # testing is possible.
    print_and_log_test_step "Waiting for USB mount."
    exit_code=0
    ($TIMEOUT ${APP_STARTUP_TIME_S}s grep -q "tud_mount_cb" <(tail -f "$TARGET_REPORT")) || exit_code=1

    if [ $exit_code -ne 0 ]; then
        print_and_log_failure "Timeout while waiting for USB mount event."
        return 1
    fi
}

function wait_for_lsusb_entry {
    print_and_log_test_step "Waiting for lsusb entry."
    while [ -z "$(lsusb -d $DFU_MODE_VID_PID)" ]; do
        sleep 1
        (( post_usb_mount_delay_s -= 1 ))
        if [ $post_usb_mount_delay_s -le 0 ]; then
            print_and_log_failure "Timeout while waiting for lsusb entry."
            return 1
        fi
    done
}

function run_cdc_tests {
    print_and_log_test_step "Writing CDC test data on both interfaces."
    ($TIMEOUT ${PY_TIMEOUT_S}s python3 "$REPO_ROOT/test/rtos_drivers/usb/serial_send_receive.py" -if0 "$SERIAL_TX0_FILE" -if1 "$SERIAL_TX1_FILE" -of0 "$SERIAL_RX0_FILE" -of1 "$SERIAL_RX1_FILE" 2>&1) >> "$HOST_REPORT"
    exit_code=$?
    if [ $exit_code -ne 0 ]; then
        print_and_log_failure "An error occurred during CDC test (exit code = $exit_code)."
        return 1
    fi
}

function verify_cdc_files {
    # Verify that the SHA for the pre-test and download files are different;
    # and then verify that the download and post-test files are the same.
    print_and_log_test_step "Verifying CDC SHA512s."

    SERIAL_TX0_FILE_SHA512=$(sha512sum "$SERIAL_TX0_FILE" | awk '{print $1}')
    SERIAL_TX1_FILE_SHA512=$(sha512sum "$SERIAL_TX1_FILE" | awk '{print $1}')
    SERIAL_RX0_FILE_SHA512=$(sha512sum "$SERIAL_RX0_FILE" | awk '{print $1}')
    SERIAL_RX1_FILE_SHA512=$(sha512sum "$SERIAL_RX1_FILE" | awk '{print $1}')

    if [ "$SERIAL_TX0_FILE_SHA512" != "$SERIAL_RX1_FILE_SHA512" ]; then
        print_and_log_failure "The SHA of the file transmitted on 0 ($SERIAL_TX0_FILE_SHA512) does not match what was received on 1 ($SERIAL_RX1_FILE_SHA512)."
        return 1
    fi

    if [ "$SERIAL_TX1_FILE_SHA512" != "$SERIAL_RX0_FILE_SHA512" ]; then
        print_and_log_failure "The SHA of the file transmitted on 1 ($SERIAL_TX0_FILE_SHA512) does not match what was received on 0 ($SERIAL_RX1_FILE_SHA512)."
        return 1
    fi
}

function run_dfu_tests {
    # First determine the state of the image currently on the target, then download
    # the test image, and then read it back a final time to verify that it changed.
    # Finally issue a DFU detach, to allow the device to terminate its application.

    print_and_log_test_step "Reading DFU initial state of device's test partition."
    dfu-util $DFU_VERBOSITY -d "$DFU_RT_VID_PID,$DFU_MODE_VID_PID" -a $DFU_ALT -U "$FILE_PRE_DOWNLOAD" >> "$HOST_REPORT" 2>&1
    exit_code=$?
    if [ $exit_code -ne 0 ]; then
        print_and_log_failure "An error occurred during upload of pre-test image (exit code = $exit_code)."
        return 1
    fi

    print_and_log_test_step "Writing DFU test image to target's test partition."
    dfu-util $DFU_VERBOSITY -d "$DFU_RT_VID_PID,$DFU_MODE_VID_PID" -a $DFU_ALT -D "$FILE_TO_DOWNLOAD" >> "$HOST_REPORT" 2>&1
    exit_code=$?
    if [ $exit_code -ne 0 ]; then
        print_and_log_failure "An error occurred during download of test image (exit code = $exit_code)."
        return 1
    fi

    print_and_log_test_step "Reading back DFU test image from the device's test partition."
    dfu-util $DFU_VERBOSITY -d "$DFU_RT_VID_PID,$DFU_MODE_VID_PID" -a $DFU_ALT -U "$FILE_POST_DOWNLOAD" >> "$HOST_REPORT" 2>&1
    exit_code=$?
    if [ $exit_code -ne 0 ]; then
        print_and_log_failure "An error occurred during upload of post-test image (exit code = $exit_code)."
        return 1
    fi

    print_and_log_test_step "Issuing DFU detach request."
    dfu-util $DFU_VERBOSITY -e -d "$DFU_RT_VID_PID,$DFU_MODE_VID_PID" -a $DFU_ALT >> "$HOST_REPORT" 2>&1
    exit_code=$?
    if [ $exit_code -ne 0 ]; then
        print_and_log_failure "An error occurred during DFU detach request (exit code = $exit_code)."
        return 1
    fi

    sleep $APP_SHUTDOWN_TIME_S
}

function verify_dfu_files {
    # Verify that the SHA for the pre-test and download files are different;
    # and then verify that the download and post-test files are the same.
    print_and_log_test_step "Verifying DFU SHA512s."

    FILE_PRE_DOWNLOAD_SHA512=$(sha512sum "$FILE_PRE_DOWNLOAD" | awk '{print $1}')
    FILE_TO_DOWNLOAD_SHA512=$(sha512sum "$FILE_TO_DOWNLOAD" | awk '{print $1}')
    FILE_POST_DOWNLOAD_SHA512=$(sha512sum "$FILE_POST_DOWNLOAD" | awk '{print $1}')

    if [ "$FILE_PRE_DOWNLOAD_SHA512" == "$FILE_TO_DOWNLOAD_SHA512" ]; then
        print_and_log_failure "The test setup is in an indeterminate state; the SHA of the download file ($FILE_TO_DOWNLOAD_SHA512) matches the pre-test state ($FILE_PRE_DOWNLOAD_SHA512)."
        return 1
    fi

    if [ "$FILE_TO_DOWNLOAD_SHA512" != "$FILE_POST_DOWNLOAD_SHA512" ]; then
        print_and_log_failure "The SHA of the post-test state ($FILE_POST_DOWNLOAD_SHA512) does not match what was downloaded ($FILE_TO_DOWNLOAD_SHA512)."
        return 1
    fi
}

trap cleanup EXIT

child_process_pid=0

rm -rf "$TEST_DIR"
mkdir -p $TEST_DIR
mkfifo "$TMP_FIFO"

echo "****************"
echo "* Run Tests    *"
echo "****************"

print_and_log_test_step "Generating test files."
generate_test_file $FILE_TO_DOWNLOAD $DFU_DOWNLOAD_BYTES
generate_test_file $SERIAL_TX0_FILE $SERIAL_TEST_DATA_BYTES
generate_test_file $SERIAL_TX1_FILE $SERIAL_TEST_DATA_BYTES

# Start the target application as a background process.
run_target

wait_for_usb_mount

if [ $exit_code -eq 0 ]; then
    wait_for_lsusb_entry
    exit_code=$?
fi

if [ $exit_code -eq 0 ]; then
    run_cdc_tests
    exit_code=$?
fi

if [ $exit_code -eq 0 ]; then
    verify_cdc_files
    exit_code=$?
fi

if [ $exit_code -eq 0 ]; then
    run_dfu_tests
    exit_code=$?
fi

if [ $exit_code -eq 0 ]; then
    verify_dfu_files
    exit_code=$?
fi

if [ $exit_code -eq 0 ]; then
    echo "[TEST PASS]: All host checks completed successfully." >> "$HOST_REPORT"
fi
