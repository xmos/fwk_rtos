
## merge_binaries combines multiple xcore applications into one by extracting
## a tile elf and recombining it into another binary.
## This macro takes an output target name, a base target, a target
## containing a tile to merge, and the tile number to merge.
## The resulting output will be a target named _OUTPUT_TARGET_NAME, which
## contains the _BASE_TARGET application with tile _TILE_TO_MERGE replaced with
## the respective tile from _OTHER_TARGET.
macro(merge_binaries _OUTPUT_TARGET_NAME _BASE_TARGET _OTHER_TARGET _TILE_NUM_TO_MERGE)
    get_target_property(BASE_TILE_DIR     ${_BASE_TARGET}  BINARY_DIR)
    get_target_property(BASE_TILE_NAME    ${_BASE_TARGET}  NAME)
    get_target_property(OTHER_TILE_NAME   ${_OTHER_TARGET} NAME)

    add_custom_target(${_OUTPUT_TARGET_NAME}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OTHER_TILE_NAME}_split
        COMMAND xobjdump --split --split-dir ${OTHER_TILE_NAME}_split ${OTHER_TILE_NAME}.xe
        COMMAND xobjdump ${BASE_TILE_NAME}.xe -r 0,${_TILE_NUM_TO_MERGE},${OTHER_TILE_NAME}_split/image_n0c${_TILE_NUM_TO_MERGE}_2.elf
        COMMAND ${CMAKE_COMMAND} -E copy ${BASE_TILE_NAME}.xe ${_OUTPUT_TARGET_NAME}.xe
        DEPENDS
            ${_BASE_TARGET}
            ${_OTHER_TARGET}
        BYPRODUCTS
            ${OTHER_TILE_NAME}_split
        WORKING_DIRECTORY
            ${BASE_TILE_DIR}
        COMMENT
            "Merge tile ${_TILE_NUM_TO_MERGE} of ${_OTHER_TARGET}.xe into ${_BASE_TARGET}.xe to create ${_OUTPUT_TARGET_NAME}.xe"
        VERBATIM
    )
    set_target_properties(${_OUTPUT_TARGET_NAME} PROPERTIES BINARY_DIR ${BASE_TILE_DIR})
endmacro()

## Creates a run target for a provided binary
macro(create_run_target _EXECUTABLE_TARGET_NAME)
    add_custom_target(run_${_EXECUTABLE_TARGET_NAME}
      COMMAND xrun --xscope ${_EXECUTABLE_TARGET_NAME}.xe
      DEPENDS ${_EXECUTABLE_TARGET_NAME}
      COMMENT
        "Run application"
      VERBATIM
    )
endmacro()

## Creates a debug target for a provided binary
macro(create_debug_target _EXECUTABLE_NAME)
    add_custom_target(debug_${_EXECUTABLE_NAME}
      COMMAND xgdb ${_EXECUTABLE_NAME}.xe -ex "connect" -ex "connect --xscope" -ex "run"
      DEPENDS ${_EXECUTABLE_NAME}.xe
      COMMENT
        "Debug application"
    )
endmacro()

## Creates a filesystem file for a provided binary
##   filename must end in "_fat.fs"
macro(create_filesystem_target _EXECUTABLE_NAME)
    add_custom_target(make_fs_${_EXECUTABLE_NAME}
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/${_EXECUTABLE_NAME}_fat.fs ${_EXECUTABLE_NAME}_fat.fs
      DEPENDS ${_EXECUTABLE_NAME}_fat.fs
      COMMENT
        "Make filesystem"
    )
endmacro()

## Creates a flash app target for a provided binary
macro(create_flash_app_target _EXECUTABLE_NAME)
    add_custom_target(flash_app_${_EXECUTABLE_NAME}
      COMMAND xflash --quad-spi-clock 50MHz ${_EXECUTABLE_NAME}.xe
      DEPENDS ${_EXECUTABLE_NAME}.xe
      COMMENT
        "Flash application"
    )
endmacro()

## Query the version of the XTC Tools
##
##   Populates the following variables:
##
##     XTC_VERSION_MAJOR
##     XTC_VERSION_MINOR
##     XTC_VERSION_PATCH
function(query_tools_version)
    # Run xcc --version
    execute_process(
        COMMAND xcc --version
        OUTPUT_VARIABLE XCC_VERSION_OUTPUT_STRING
    )
    # Split output into lines
    string(REPLACE "\n" ";" XCC_VERSION_OUTPUT_LINES ${XCC_VERSION_OUTPUT_STRING})
    # Get second line
    list(GET XCC_VERSION_OUTPUT_LINES 1 XCC_VERSION_LINE)
    message(STATUS ${XCC_VERSION_LINE})
    # Parse version fields
    string(SUBSTRING ${XCC_VERSION_LINE} 13 10 XCC_SEMVER)
    string(REPLACE "." ";" XCC_SEMVER_FIELDS ${XCC_SEMVER})
    list(LENGTH XCC_SEMVER_FIELDS XCC_SEMVER_FIELDS_LENGTH)
    if (${XCC_SEMVER_FIELDS_LENGTH} EQUAL "3")
        list(GET XCC_SEMVER_FIELDS 0 XCC_VERSION_MAJOR)
        list(GET XCC_SEMVER_FIELDS 1 XCC_VERSION_MINOR)
        list(GET XCC_SEMVER_FIELDS 2 XCC_VERSION_PATCH)
        # Set XTC version env variables
        set(XTC_VERSION_MAJOR ${XCC_VERSION_MAJOR} PARENT_SCOPE)
        set(XTC_VERSION_MINOR ${XCC_VERSION_MINOR} PARENT_SCOPE)
        set(XTC_VERSION_PATCH ${XCC_VERSION_PATCH} PARENT_SCOPE)
    else()
        # Unable to determine the version. Return 15.1.0 and hope for the best
        # Note, 15.1.0 had a bug where the version was missing
        set(XTC_VERSION_MAJOR "15" PARENT_SCOPE)
        set(XTC_VERSION_MINOR "1" PARENT_SCOPE)
        set(XTC_VERSION_PATCH "0" PARENT_SCOPE)
    endif()
endfunction()