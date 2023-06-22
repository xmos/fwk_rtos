#!/bin/bash
# Copyright (c) 2021-2023, XMOS Ltd, All rights reserved
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

set -e # exit on first error
set -x

# help text
help()
{
   echo "Framework RTOS Drivers Wifi test"
   echo
   echo "Syntax: check_wifi.sh [-h] adapterID_optional"
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
FIRMWARE=test_rtos_driver_wifi.xe
TIMEOUT_S=60
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

echo "********************"
echo "* Build Filesystem *"
echo "********************"
pushd ${REPO_ROOT}/test/rtos_drivers/wifi
cd ${REPO_ROOT}/test/rtos_drivers/wifi/filesystem_support
if [ ! -f ${REPO_ROOT}/test/rtos_drivers/wifi/filesystem_support/fat.fs ]; then
    ./create_fs.sh
fi
popd

echo "****************"
echo "* Flash Device *"
echo "****************"
xflash ${ADAPTER_ID} --force --quad-spi-clock 50MHz --factory ${REPO_ROOT}/dist/test_rtos_driver_wifi.xe --boot-partition-size 0x100000 --data ${REPO_ROOT}/test/rtos_drivers/wifi/filesystem_support/fat.fs

echo "*************"
echo "* Run Tests *"
echo "*************"
if [ "$UNAME" == "Linux" ] ; then
    timeout ${TIMEOUT_S}s xrun --xscope ${ADAPTER_ID} ${REPO_ROOT}/dist/${FIRMWARE} 2>&1 | tee -a ${REPORT}
elif [ "$UNAME" == "Darwin" ] ; then
    gtimeout ${TIMEOUT_S}s xrun --xscope ${ADAPTER_ID} ${REPO_ROOT}/dist/${FIRMWARE} 2>&1 | tee -a ${REPORT}
fi

echo "****************"
echo "* Parse Result *"
echo "****************"
python ${REPO_ROOT}/test/rtos_drivers/python/parse_test_output.py testing/test.rpt -outfile="testing/test_results" --print_test_results --verbose
