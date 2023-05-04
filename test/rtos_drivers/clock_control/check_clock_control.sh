#!/bin/bash
# Copyright (c) 2023, XMOS Ltd, All rights reserved
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

set -e # exit on first error

# help text
help()
{
   echo "Framework RTOS Drivers Hardware in the Loop test"
   echo
   echo "Syntax: check_drivers_hil.sh [-h] adapterID_optional"
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
FIRMWARE=test_rtos_driver_clock_control_test.xe
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

echo "*************"
echo "* Run Tests *"
echo "*************"
if [ "$UNAME" == "Linux" ] || [ -n "$MSYSTEM" ]; then
    timeout ${TIMEOUT_S}s xrun --xscope ${REPO_ROOT}/dist/${FIRMWARE} 2>&1 | tee -a ${REPORT}
elif [ "$UNAME" == "Darwin" ] ; then
    gtimeout ${TIMEOUT_S}s xrun --xscope ${REPO_ROOT}/dist/${FIRMWARE} 2>&1 | tee -a ${REPORT}
fi

echo "****************"
echo "* Parse Result *"
echo "****************"

echo "**Currently Not Implemented**"
