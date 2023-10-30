#!/bin/bash
set -e

if [ -f /.dockerenv ]; then
    # Docker workaround for: "fatal: detected dubious ownership in repository"
    git config --global --add safe.directory /fwk_rtos
fi

REPO_ROOT=$(git rev-parse --show-toplevel)

source ${REPO_ROOT}/tools/ci/helper_functions.sh
export_ci_build_vars

# setup distribution folder
DIST_DIR=${REPO_ROOT}/dist_host
mkdir -p ${DIST_DIR}

# row format is: "target     copy_path"
applications=(
    "fatfs_mkimage   tools/fatfs_mkimage"
    "datapartition_mkimage   tools/datapartition_mkimage"
)

# perform builds
path="${REPO_ROOT}"
echo '******************************************************'
echo '* Building host applications'
echo '******************************************************'

(cd ${path}; rm -rf build_host)
(cd ${path}; mkdir -p build_host)
(cd ${path}/build_host; log_errors cmake ../ -G "$CI_CMAKE_GENERATOR")

for ((i = 0; i < ${#applications[@]}; i += 1)); do
    read -ra FIELDS <<< ${applications[i]}
    target="${FIELDS[0]}"
    copy_path="${FIELDS[1]}"
    (cd ${path}/build_host; log_errors $CI_BUILD_TOOL ${target} $CI_BUILD_TOOL_ARGS)
    (cd ${path}/build_host; cp ${copy_path}/${target} ${DIST_DIR})
done