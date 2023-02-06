#!/bin/bash
set -e

REPO_ROOT=$(git rev-parse --show-toplevel)

source ${REPO_ROOT}/tools/ci/helper_functions.sh

BUILD_DIR="${REPO_ROOT}/dist_host"
APPLICATION="datapartition_mkimage"
APP=${BUILD_DIR}/${APPLICATION}
APP_LOG=${APPLICATION}.log

TIMEOUT_EXE=$(get_timeout)
DURATION="10s"

DEFAULT_BLOCK_SIZE_INT=512
DEFAULT_BLOCK_SIZE_HEX=$(printf "0x%x\n" "$DEFAULT_BLOCK_SIZE_INT")
DEFAULT_SEEK_BLOCK_INT=0
DEFAULT_SEEK_BLOCK_HEX=$(printf "0x%x\n" "$DEFAULT_SEEK_BLOCK_INT")
DEFAULT_FILL_BYTE_INT=255
DEFAULT_FILL_BYTE_HEX=$(printf "0x%x\n" "$DEFAULT_FILL_BYTE_INT")
OUT_FILE=output.txt

TMP_DIR=$(mktemp -d)
trap 'rm -rf -- "$TMP_DIR"' EXIT
cd "$TMP_DIR"

TEST_FILE_A="$TMP_DIR/file-A.txt"
TEST_FILE_B="./file B.txt"
TEST_FILE_C="fileC"
STDIN_IN_STR="\[STDIN\]" # Escape chars needed for RegEx handling.

cmd_count=0

function run_command {
    ((cmd_count++)) || true
    echo ""
    echo "COMMAND #$cmd_count ($cmd)"
    eval "($TIMEOUT_EXE $DURATION $cmd 2>&1 | tee $APP_LOG)"
}

function verify_sha512 {
    echo "$EXPECTED_OUT_FILE_SHA512 $OUT_FILE" > "$OUT_FILE.sha512"
    result=$(sha512sum -c "$OUT_FILE.sha512")
    result_ok=$(echo "$result" | grep -c "$OUT_FILE: OK" || true)

    if [ "$result_ok" -ne 1 ]; then
        # If there is a need to update a hash, run: "sha512sum $OUT_FILE"
        echo "COMMAND #$cmd_count FAIL: SHA512 does not match."
        exit 1
    fi
}

function verify_filesize_log {
    old_size_log=$(printf "Filesize (old): %d Bytes" "$expected_old_filesize")
    new_size_log=$(printf "Filesize (new): %d Bytes" "$expected_new_filesize")
    result_old_size=$(grep -c "$old_size_log" $APP_LOG || true)
    result_new_size=$(grep -c "$new_size_log" $APP_LOG || true)
    if [ "$result_old_size" -ne 1 ] || [ "$result_new_size" -ne 1 ]; then
        echo "COMMAND #$cmd_count FAIL: Unexpected filesize(s)."
        exit 1
    fi
}

function verify_no_error_log {
    result_errors=$(grep -c "ERROR" $APP_LOG || true)

    if [ "$result_errors" -ne 0 ]; then
        echo "COMMAND #$cmd_count FAIL: Errors(s) present."
        exit 1
    fi
}

function verify_no_warning_log {
    result_warnings=$(grep -c "WARNING" $APP_LOG || true)

    if [ "$result_warnings" -ne 0 ]; then
        echo "COMMAND #$cmd_count FAIL: Warning(s) present."
        exit 1
    fi
}

function verify_file_entry_log {
    result_entry=$(grep -c "$expected_file_log_entry" $APP_LOG || true)

    if [ "$result_entry" -ne "1" ]; then
        echo "COMMAND #$cmd_count FAIL: Unexpected file entry."
        exit 1
    fi
}

# Populate the input test files with data that can be used to uniquely identify them im the output results.
echo -n "A: 11 chars" > "$TEST_FILE_A"
echo -n "B:  12 chars" > "$TEST_FILE_B"
echo -n "C:   13 chars" > $TEST_FILE_C

MIN_BLOCK_SIZE=13 # Based on the largest input source above.

# Populate the output file with some data. Verifies overwrite behavior of application.
# Input file data length should be less than this to observe.
echo -n "0123456789ABCDEF0123456789ABCDEF" > $OUT_FILE

# Sorted inputs according to the input's block number (within in_blocks).
# This simplifies output log validation. The test matrix should exercise unsorted arrangements.
in_blocks=(0 1 4)
in_files=("$TEST_FILE_A" "$TEST_FILE_B" "$TEST_FILE_C")

#
# TEST CASE
# Test input arrangements, value formats, default values.
#

test_case_commands=(
    "cat ${in_files[0]} | $APP -v -t -i \"${in_files[1]}:${in_blocks[1]}\" ${in_files[2]}:${in_blocks[2]} -o \"$OUT_FILE\"" # This entry must be first, otherwise logic specifically handling STDIN_IN_STR needs to be changed.
    "echo -n \"\" | $APP -v -b $DEFAULT_BLOCK_SIZE_INT -f $DEFAULT_FILL_BYTE_INT -s $DEFAULT_SEEK_BLOCK_INT -o \"$OUT_FILE\" -i \"${in_files[0]}:${in_blocks[0]}\" \"${in_files[1]}:${in_blocks[1]}\" ${in_files[2]}:${in_blocks[2]}"
    "$APP -v -b $DEFAULT_BLOCK_SIZE_INT -f $DEFAULT_FILL_BYTE_INT -s $DEFAULT_SEEK_BLOCK_INT -o \"$OUT_FILE\" -i \"${in_files[0]}:${in_blocks[0]}\" \"${in_files[1]}:${in_blocks[1]}\" ${in_files[2]}:${in_blocks[2]}"
    "$APP -v -b $DEFAULT_BLOCK_SIZE_HEX -f $DEFAULT_FILL_BYTE_HEX -s $DEFAULT_SEEK_BLOCK_HEX -o \"$OUT_FILE\" -i \"${in_files[1]}:${in_blocks[1]}\" ${in_files[2]}:${in_blocks[2]} \"${in_files[0]}:${in_blocks[0]}\""
)

for cmd in "${test_case_commands[@]}"
do :
    expected_old_filesize=$(stat -c%s "$OUT_FILE")
    run_command

    # Verify output file via sha512
    # If there is a need to update this hash, run: "sha512sum $OUT_FILE"
    EXPECTED_OUT_FILE_SHA512="7ab97c1eef00a73862b426b5cb1e5ed100e9620bab189b9c5fa106161058d5822d79da2d915f22ba0e356c974b8dcaa08e5cc74ff0b108d46f884dc9c8733670"
    verify_sha512

    i=0

    # Verify log output's table and check that no errors/warnings occurred.
    for in_file in "${in_files[@]}"
    do :
        file_size=$(stat -c%s "$in_file")
        if [ $cmd_count -eq 1 ] && [ "$i" -eq "0" ]; then
            in_file=$STDIN_IN_STR
        fi

        file_offset=$((DEFAULT_BLOCK_SIZE_INT * ${in_blocks[$i]}))
        expected_file_log_entry=$(printf "0x%08X\s*%d\s*%s" "$file_offset" "$file_size" "$in_file")
        verify_file_entry_log

        ((i++)) || true
    done

    # Verify output filesize reporting is correct, and no errors/warniings are
    # reported. The last file in the prior loop determines the size.
    expected_new_filesize=$((file_offset + file_size))
    verify_filesize_log
    verify_no_error_log
    verify_no_warning_log
done

#
# TEST CASE
# Verify truncation, non-default block size/offet, non-default fill byte,
# and the long-form of argument options.
#

fill_byte=0x55
block_offset=4
block_size=256

# Reinitialize the output file using non-default fill.
rm -f $OUT_FILE
cmd="echo -n \"\" | $APP --verbose --fill-byte $fill_byte --block-size $block_size --seek $block_offset --out-file \"$OUT_FILE\""
run_command

# Verify initial filesize matches expectation.
actual_filesize=$(stat -c%s "$OUT_FILE")
expected_filesize=$((block_offset * block_size))

if [ "$expected_filesize" -ne "$actual_filesize" ]; then
    echo "COMMAND #$cmd_count FAIL"
    exit 1
fi

expected_old_filesize=$actual_filesize
block_offset=2
block_size=$MIN_BLOCK_SIZE

# Specify input data that effectively writes to addresses smaller than the
# current filesize to observe truncation. Using $MIN_BLOCK_SIZE for the block
# size verifies that minimum block size completes without error.
cmd="cat $TEST_FILE_A | $APP --verbose --truncate --block-size $block_size --seek $block_offset --in-file \"$TEST_FILE_C:1\" --out-file \"$OUT_FILE\""
run_command

# Verify that truncation occurred and no errors/warnings reported.
file_size=$(stat -c%s "$TEST_FILE_A")
expected_new_filesize=$((block_size * block_offset + file_size))

verify_filesize_log
verify_no_error_log
verify_no_warning_log

# Verify output file via sha512
EXPECTED_OUT_FILE_SHA512="13098e61ef9137517bb017a77579aea68da742784d1e802002c5d8a1d8884c98db4639cbd52a9cd780b374fac50a0ec4284bf600a84c1d7fd6c354faa92cb6c1"
verify_sha512

#
# TEST CASE
# Verify that truncation does not occur when -t is omitted.
#

cmd="echo -n \"\" | $APP --verbose --out-file \"$OUT_FILE\""
run_command

# File size reported should be unchanged.
expected_old_filesize=$expected_new_filesize

verify_filesize_log
verify_no_error_log
verify_no_warning_log
verify_sha512

#
# TEST CASE
# Verify error is reported when overlapping input data is detected.
#

block_size=$((MIN_BLOCK_SIZE - 1))
cmd="cat $TEST_FILE_C | $APP -v -b $block_size -i \"$TEST_FILE_C:2\" \"$TEST_FILE_A:3\" -o \"$OUT_FILE\""
run_command

result_errors=$(grep -c "ERROR: Overlapping input data detected." $APP_LOG || true)
if [ "$result_errors" -ne 1 ]; then
    echo "COMMAND #$cmd_count FAIL"
    exit 1
fi
