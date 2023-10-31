#!/bin/bash
# Copyright (c) 2023, XMOS Ltd, All rights reserved
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

set -e # exit on first error

if [ -f /.dockerenv ]; then
    # Docker workaround for: "fatal: detected dubious ownership in repository"
    git config --global --add safe.directory /fwk_rtos
fi

# discern repo root
REPO_ROOT=`git rev-parse --show-toplevel`
source ${REPO_ROOT}/tools/ci/helper_functions.sh
export_ci_build_vars

# setup distribution folder
DIST_DIR=${REPO_ROOT}/dist
mkdir -p ${DIST_DIR}

# setup configurations
# row format is: "make_target BOARD toolchain"
applications=(
    "test_rtos_driver_hil                 XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    "test_rtos_driver_hil_add             XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    "test_rtos_driver_wifi                XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    "test_rtos_driver_usb                 XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    # "test_rtos_driver_clock_control_test  XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
)

# perform builds
for ((i = 0; i < ${#applications[@]}; i += 1)); do
    read -ra FIELDS <<< ${applications[i]}
    make_target="${FIELDS[0]}"
    board="${FIELDS[1]}"
    toolchain_file="${REPO_ROOT}/${FIELDS[2]}"
    path="${REPO_ROOT}"
    echo '******************************************************'
    echo '* Building' ${make_target} 'for' ${board}
    echo '******************************************************'

    (cd ${path}; rm -rf build_${board})
    (cd ${path}; mkdir -p build_${board})
    (cd ${path}/build_${board}; log_errors cmake ../ -G "$CI_CMAKE_GENERATOR" --toolchain=${toolchain_file} -DBOARD=${board} -DFRAMEWORK_RTOS_TESTS=ON; log_errors $CI_BUILD_TOOL ${make_target} $CI_BUILD_TOOL_ARGS)
    (cd ${path}/build_${board}; cp ${make_target}.xe ${DIST_DIR})
done
