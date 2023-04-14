#!/bin/bash
# Copyright 2021-2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

set -e

REPO_ROOT=$(git rev-parse --show-toplevel)
TEST_DIR=testing
TMP_FIFO=$TEST_DIR/pid
TARGET_REPORT=$TEST_DIR/target.rpt
HOST_REPORT=$TEST_DIR/host.rpt
FILE_PRE_DOWNLOAD=$TEST_DIR/pre_download_data.bin
FILE_TO_DOWNLOAD=$TEST_DIR/download_data.bin
FILE_POST_DOWNLOAD=$TEST_DIR/post_download_data.bin
FIRMWARE=$REPO_ROOT/dist/test_rtos_driver_usb.xe
DFU_RT_VID_PID=""
DFU_MODE_VID_PID="20b1:4000"
DFU_DOWNLOAD_BYTES=248
DFU_ALT=1

APP_STARTUP_TIME_S=5
APP_SHUTDOWN_TIME_S=5

DFU_VERBOSITY="" # Set to "-v -v -v" libusb debug prints.

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

function cleanup {
    rm -f "$TMP_FIFO.lnk"

    if [ $child_process_pid -gt 0 ]; then
        child_process_running=$(ps -p $child_process_pid | grep $child_process_pid)
        if [ -n "$child_process_running" ]; then
            print_and_log_test_step "Stopping child process."
            kill $child_process_pid
        fi
    fi
}

function generate_test_file {
    # Generate a file populated with urandom data.
    #head -c $DFU_DOWNLOAD_BYTES < /dev/urandom > "$FILE_TO_DOWNLOAD"
    NUM_HASH_CHARS=32 # MD5 is 32-chars
    bytes_to_generate=$DFU_DOWNLOAD_BYTES

    while [ $bytes_to_generate -gt $NUM_HASH_CHARS ]; do
        x=$(echo $RANDOM | md5sum | head -c $NUM_HASH_CHARS)
        (( bytes_to_generate -= NUM_HASH_CHARS ))
        echo -n "$x" >> "$FILE_TO_DOWNLOAD"
    done

    x=$(echo $RANDOM | md5sum | head -c $bytes_to_generate)
    echo -n "$x" >> "$FILE_TO_DOWNLOAD"
}

function run_dfu_target {
    echo "----------"
    echo "Target Log"
    echo "----------"
    print_and_log_test_step "Starting DFU target application."
    (xrun --xscope "$FIRMWARE" 2>&1 & echo $! > $TMP_FIFO) | tee -a "$TARGET_REPORT" &
    child_process_pid=$(<$TMP_FIFO)
}

function run_dfu_host {
    # First determine the state of the image currently on the target, then download
    # the test image, and then read it back a final time to verify that it changed.
    # Finally issue a DFU detach, to allow the device to terminate its application.

    # NOTE: The commands below are piped into tee instead of >>FILE so that the
    # dfu-util's exit code can be captured instead of the result of the stream
    # redirection.

    print_and_log_test_step "Reading initial state of device's test partition."
    dfu-util $DFU_VERBOSITY -d "$DFU_RT_VID_PID,$DFU_MODE_VID_PID" -a $DFU_ALT -U "$FILE_PRE_DOWNLOAD" 2>&1 | tee -a "$HOST_REPORT" > /dev/null
    exit_code=${PIPESTATUS[0]}
    if [ $exit_code -ne 0 ]; then
        print_and_log_failure "An error occurred during upload of pre-test image (exit code = $exit_code)."
        return 1
    fi

    print_and_log_test_step "Writing test image to target's test partition."
    dfu-util $DFU_VERBOSITY -d "$DFU_RT_VID_PID,$DFU_MODE_VID_PID" -a $DFU_ALT -D "$FILE_TO_DOWNLOAD" 2>&1 | tee -a "$HOST_REPORT" > /dev/null
    exit_code=${PIPESTATUS[0]}
    if [ $exit_code -ne 0 ]; then
        print_and_log_failure "An error occurred during download of test image (exit code = $exit_code)."
        return 1
    fi

    print_and_log_test_step "Reading back test image from the device's test partition."
    dfu-util $DFU_VERBOSITY -d "$DFU_RT_VID_PID,$DFU_MODE_VID_PID" -a $DFU_ALT -U "$FILE_POST_DOWNLOAD" 2>&1 | tee -a "$HOST_REPORT" > /dev/null
    exit_code=${PIPESTATUS[0]}
    if [ $exit_code -ne 0 ]; then
        print_and_log_failure "An error occurred during upload of post-test image (exit code = $exit_code)."
        return 1
    fi

    print_and_log_test_step "Issuing DFU detach request."
    dfu-util $DFU_VERBOSITY -e -d "$DFU_RT_VID_PID,$DFU_MODE_VID_PID" -a $DFU_ALT 2>&1 | tee -a "$HOST_REPORT" > /dev/null
    exit_code=${PIPESTATUS[0]}
    if [ $exit_code -ne 0 ]; then
        print_and_log_failure "An error occurred during DFU detach request (exit code = $exit_code)."
        return 1
    fi

    sleep $APP_SHUTDOWN_TIME_S
}

function verify_dfu_files {
    # Verify that the SHA for the pre-test and download files are different;
    # and then verify that the download and post-test files are the same.
    print_and_log_test_step "Verifying SHA512s."

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

rm -f "$TARGET_REPORT"
rm -f "$HOST_REPORT"

rm -f "$FILE_PRE_DOWNLOAD"
rm -f "$FILE_TO_DOWNLOAD"
rm -f "$FILE_POST_DOWNLOAD"

echo "****************"
echo "* Run Tests    *"
echo "****************"

generate_test_file

# Start the target application as a background process.
run_dfu_target

# Waiting for application to start
sleep $APP_STARTUP_TIME_S

run_dfu_host
exit_code=$?

if [ $exit_code -eq 0 ]; then
    verify_dfu_files
    exit_code=$?
fi

if [ $exit_code -eq 0 ]; then
    echo "[TEST PASS]: All host checks completed successfully." >> "$HOST_REPORT"
fi

echo "****************"
echo "* Parse Result *"
echo "****************"
# If the script gets to this point, all host side checks have passed.
# As a final step, verify that the target logs report a passing result for each
# tile.
result_target=$(grep -c "PASS" "$TARGET_REPORT" || true)

if [ $result_target -lt 2 ]; then
    echo "FAIL"
    exit 1
fi

echo "PASS"
