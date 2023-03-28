    ## Import dependencies
    include(FetchContent)

    IF(EXISTS ${CMAKE_BINARY_DIR}/dependencies/fwk_io)
        message(STATUS "Skipped: dependencies/fwk_io")
    else()
        message(STATUS "Fetching: dependencies/fwk_io")
        FetchContent_Declare(
            fwk_io
            GIT_REPOSITORY https://github.com/xmos/fwk_io.git
            GIT_TAG        4378d2370834b482e4fcce8ee7d51b1c68dd3da4
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
            GIT_TAG        9e4f6196386995e2d7786b376091404638055639
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
            GIT_TAG        b0e3fd1b9cc0582310cb06959e33ee0602774efa
            GIT_SHALLOW    FALSE
            GIT_SUBMODULES_RECURSE TRUE
            SOURCE_DIR     ${CMAKE_BINARY_DIR}/dependencies/lib_qspi_fast_read
        )
        FetchContent_Populate(lib_qspi_fast_read)
    endif()

    add_subdirectory(${CMAKE_BINARY_DIR}/dependencies/lib_qspi_fast_read)