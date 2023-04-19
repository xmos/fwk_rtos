#!/bin/bash
# Copyright 2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

set -e

# Get unix name for determining OS
UNAME=$(uname)

rm -rf testing
mkdir testing
REPORT=testing/test.rpt
FIRMWARE=rtos_drivers_wifi.xe
TIMEOUT_S=60

REPO_ROOT=`git rev-parse --show-toplevel`

rm -f ${REPORT}

echo "****************"
echo "* Flash Device *"
echo "****************"
pushd .
cd filesystem_support
./flash_image.sh
popd

echo "****************"
echo "* Run Tests    *"
echo "****************"
if [ "$UNAME" == "Linux" ] || [ -n "$MSYSTEM" ]; then
    timeout ${TIMEOUT_S}s xrun --xscope bin/${FIRMWARE} 2>&1 | tee -a ${REPORT}
elif [ "$UNAME" == "Darwin" ] ; then
    gtimeout ${TIMEOUT_S}s xrun --xscope bin/${FIRMWARE} 2>&1 | tee -a ${REPORT}
fi

echo "****************"
echo "* Parse Result *"
echo "****************"
python ${REPO_ROOT}/test/rtos_drivers/python/parse_test_output.py testing/test.rpt -outfile="testing/test_results" --print_test_results --verbose

pytest
