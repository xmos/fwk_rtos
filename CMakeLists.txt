cmake_minimum_required(VERSION 3.21)

## Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Project declaration
project(framework_rtos)

## Enable languages for project
enable_language(CXX C ASM)

## Project options
option(FRAMEWORK_RTOS_TESTS     "Enable framework_core tests"  OFF)

## Setup a framework root path
set(FRAMEWORK_RTOS_ROOT_PATH ${PROJECT_SOURCE_DIR} CACHE STRING "Root folder of framework_rtos in this cmake project tree")

## These must be included before adding the modules subdirectory
if(FRAMEWORK_RTOS_TESTS)
    ## Import some helpful macros
    include(tools/cmake_utils/xmos_macros.cmake)
endif()

## These must be included before adding the modules subdirectory
if(FRAMEWORK_RTOS_TESTS OR PROJECT_IS_TOP_LEVEL)
    ## Fetch dependencies if top level
    include(tools/ci/fetch_ci_deps.cmake)
endif()

## Add library usage targets
add_subdirectory(modules)

## Add host tools targets
add_subdirectory(tools)

if(FRAMEWORK_RTOS_TESTS)
    include(test/tests.cmake)
endif()
