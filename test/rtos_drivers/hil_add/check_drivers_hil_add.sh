#!/bin/bash
# Copyright (c) 2023, XMOS Ltd, All rights reserved
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

set -e # exit on first error
set -x

# help text
help()
{
   echo "Framework RTOS Drivers Hardware in the Loop test"
   echo
   echo "Syntax: check_drivers_hil_add.sh [-h] adapterID_optional"
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

# assign vars
REPORT=testing/test.rpt
FIRMWARE=dist/test_rtos_driver_hil_add.xe
TIMEOUT_S=300
if [ ! -z "${@:$OPTIND:1}" ]
then
    ADAPTER_ID="--adapter-id ${@:$OPTIND:1}"
fi

# Get unix name for determining OS
UNAME=$(uname)

# clean slate
rm -rf testing
mkdir testing
rm -f ${REPORT}

# discern repository root
REPO_ROOT=`git rev-parse --show-toplevel`

echo "*********"
echo "* Flash *"
echo "*********"
RETURN_DIR=$PWD
cd ${REPO_ROOT}/build_XCORE-AI-EXPLORER
xflash --write-all ${REPO_ROOT}/build_XCORE-AI-EXPLORER/dependencies/lib_qspi_fast_read/lib_qspi_fast_read/calibration_pattern.bin ${ADAPTER_ID} --target-file=${REPO_ROOT}/test/rtos_drivers/hil_add/XCORE-AI-EXPLORER.xn
cd ${RETURN_DIR}

echo "*************"
echo "* Run Tests *"
echo "*************"
if [ "$UNAME" == "Linux" ] || [ -n "$MSYSTEM" ]; then
    timeout ${TIMEOUT_S}s xrun --xscope ${ADAPTER_ID} ${REPO_ROOT}/${FIRMWARE} 2>&1 | tee -a ${REPORT}
elif [ "$UNAME" == "Darwin" ] ; then
    gtimeout ${TIMEOUT_S}s xrun --xscope ${ADAPTER_ID} ${REPO_ROOT}/${FIRMWARE} 2>&1 | tee -a ${REPORT}
fi

echo "****************"
echo "* Parse Result *"
echo "****************"
python ${REPO_ROOT}/test/rtos_drivers/python/parse_test_output.py testing/test.rpt -outfile="testing/test_results" --print_test_results --verbose
