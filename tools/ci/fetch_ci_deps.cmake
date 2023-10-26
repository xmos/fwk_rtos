    ## Import dependencies
    include(FetchContent)

    IF(EXISTS ${CMAKE_BINARY_DIR}/dependencies/fwk_io)
        message(STATUS "Skipped: dependencies/fwk_io")
    else()
        message(STATUS "Fetching: dependencies/fwk_io")
        FetchContent_Declare(
            fwk_io
            GIT_REPOSITORY https://github.com/xmos/fwk_io.git
            GIT_TAG        dcf1a87ddd006c328906094ec12493e604bc9b2f
            GIT_SHALLOW    FALSE
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
            GIT_TAG        c4582cb3ce4da34c8757a6f8f7df8935496038eb
            GIT_SHALLOW    FALSE
            GIT_SUBMODULES_RECURSE TRUE
            SOURCE_DIR     ${CMAKE_BINARY_DIR}/dependencies/fwk_core
        )
        FetchContent_Populate(fwk_core)
    endif()

    add_subdirectory(${CMAKE_BINARY_DIR}/dependencies/fwk_core)

    IF(EXISTS ${CMAKE_BINARY_DIR}/dependencies/lib_qspi_fast_read)
        message(STATUS "Skipped: dependencies/lib_qspi_fast_read")
    else()
        message(STATUS "Fetching: dependencies/lib_qspi_fast_read")
        FetchContent_Declare(
            lib_qspi_fast_read
            GIT_REPOSITORY git@github.com:xmos/lib_qspi_fast_read.git
            GIT_TAG        85fe54188b5f3244c744f6d2aeebd757d8f25358
            GIT_SHALLOW    FALSE
            GIT_SUBMODULES_RECURSE TRUE
            SOURCE_DIR     ${CMAKE_BINARY_DIR}/dependencies/lib_qspi_fast_read
        )
        FetchContent_Populate(lib_qspi_fast_read)
    endif()

    add_subdirectory(${CMAKE_BINARY_DIR}/dependencies/lib_qspi_fast_read)
