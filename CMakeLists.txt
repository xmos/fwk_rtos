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

## Add top level project targets
if(PROJECT_IS_TOP_LEVEL)
    ## Import some helpful macros
    include(tools/cmake_utils/xmos_macros.cmake)

    ## Import dependencies
    include(FetchContent)

    IF(EXISTS ${CMAKE_BINARY_DIR}/dependencies/fwk_io)
        message(STATUS "Skipped: dependencies/fwk_io")
    else()
        message(STATUS "Fetching: dependencies/fwk_io")
        FetchContent_Declare(
            fwk_io
            GIT_REPOSITORY https://github.com/xmos/fwk_io.git
            GIT_TAG        5576a36c7f85fd73f97b09e8a37b862e3f4771de
            GIT_SHALLOW    TRUE
            GIT_SUBMODULES_RECURSE TRUE
            SOURCE_DIR     ${CMAKE_BINARY_DIR}/dependencies/fwk_io
        )
        FetchContent_Populate(fwk_io)
    endif()

    add_subdirectory(${CMAKE_BINARY_DIR}/dependencies/fwk_io)

    IF(EXISTS ${CMAKE_BINARY_DIR}/dependencies/fwk_core)
        message(STATUS "Skipped: dependencies/fwk_core")
    else()
        message(STATUS "Fetching: dependencies/fwk_core")
        FetchContent_Declare(
            fwk_core
            GIT_REPOSITORY https://github.com/xmos/fwk_core.git
            GIT_TAG        39018b4d2455a9f3ff2ade7717484bdd059407c4
            GIT_SHALLOW    TRUE
            GIT_SUBMODULES_RECURSE TRUE
            SOURCE_DIR     ${CMAKE_BINARY_DIR}/dependencies/fwk_core
        )
        FetchContent_Populate(fwk_core)
    endif()

    add_subdirectory(${CMAKE_BINARY_DIR}/dependencies/fwk_core)
endif()

## Add library usage targets
add_subdirectory(modules)

if(FRAMEWORK_RTOS_TESTS)
    include(test/tests.cmake)
endif()
