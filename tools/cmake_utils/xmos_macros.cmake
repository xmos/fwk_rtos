include_guard(DIRECTORY)

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

    add_custom_target(${_OUTPUT_TARGET_NAME} ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OTHER_TILE_NAME}_split
        COMMAND xobjdump --split --split-dir ${OTHER_TILE_NAME}_split ${OTHER_TILE_NAME}.xe > ${OTHER_TILE_NAME}_split/output.log
        COMMAND xobjdump ${BASE_TILE_NAME}.xe -r 0,${_TILE_NUM_TO_MERGE},${OTHER_TILE_NAME}_split/image_n0c${_TILE_NUM_TO_MERGE}_2.elf >> ${OTHER_TILE_NAME}_split/output.log
        COMMAND ${CMAKE_COMMAND} -E copy ${BASE_TILE_NAME}.xe ${_OUTPUT_TARGET_NAME}.xe
        DEPENDS
            ${_BASE_TARGET}
            ${_OTHER_TARGET}
        BYPRODUCTS
            ${_OUTPUT_TARGET_NAME}.xe
        WORKING_DIRECTORY
            ${BASE_TILE_DIR}
        COMMENT
            "Merge tile ${_TILE_NUM_TO_MERGE} of ${_OTHER_TARGET}.xe into ${_BASE_TARGET}.xe to create ${_OUTPUT_TARGET_NAME}.xe"
        VERBATIM
    )
    set_target_properties(${_OUTPUT_TARGET_NAME} PROPERTIES
      BINARY_DIR ${BASE_TILE_DIR}
      ADDITIONAL_CLEAN_FILES "${OTHER_TILE_NAME}_split"
    )
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

## Creates a run target for a provided binary. The first argument specifies the file to save to (no extension).
macro(create_run_xscope_to_file_target _EXECUTABLE_TARGET_NAME _XSCOPE_FILE)
    add_custom_target(run_xscope_to_file_${_EXECUTABLE_TARGET_NAME}
      COMMAND xrun --xscope-file ${_XSCOPE_FILE} ${_EXECUTABLE_TARGET_NAME}.xe
      DEPENDS ${_EXECUTABLE_TARGET_NAME}
      COMMENT
        "Run application"
      VERBATIM
    )
endmacro()

## Creates a debug target for a provided binary
macro(create_debug_target _EXECUTABLE_TARGET_NAME)
    add_custom_target(debug_${_EXECUTABLE_TARGET_NAME}
      COMMAND xgdb ${_EXECUTABLE_TARGET_NAME}.xe -ex "connect" -ex "connect --xscope" -ex "run"
      DEPENDS ${_EXECUTABLE_TARGET_NAME}
      COMMENT
        "Debug application"
    )
endmacro()

## Creates a filesystem file for a provided binary
##   filename must end in "_fat.fs"
## Optional arguments can be used to specify other dependency targets, such as filesystem generators
## create_filesystem_target(_EXECUTABLE_TARGET_NAME _FILESYSTEM_INPUT_DIR _IMAGE_SIZE _OPTIONAL_DEPENDS_TARGETS)
macro(create_filesystem_target)
  if(${ARGC} EQUAL 1)
    add_custom_target(make_fs_${ARGV0} ALL
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/${ARGV0}_fat.fs ${ARGV0}_fat.fs
      DEPENDS ${ARGV0}_fat.fs
      COMMENT
        "Move filesystem"
      VERBATIM
    )
  elseif(${ARGC} EQUAL 3)
    add_custom_target(make_fs_${ARGV0} ALL
      COMMAND fatfs_mkimage --input=${ARGV1} --image_size=${ARGV2} --output=${ARGV0}_fat.fs
      BYPRODUCTS
        ${ARGV0}_fat.fs
      COMMENT
        "Create filesystem"
      VERBATIM
    )
  elseif(${ARGC} EQUAL 4)
    add_custom_target(make_fs_${ARGV0} ALL
      COMMAND fatfs_mkimage --input=${ARGV1} --image_size=${ARGV2} --output=${ARGV0}_fat.fs
      DEPENDS ${ARGV3}
      BYPRODUCTS
        ${ARGV0}_fat.fs
      COMMENT
        "Create filesystem"
      VERBATIM
    )
  else()
    message(FATAL_ERROR "Invalid number of arguments passed to create_filesystem_target")
  endif()
endmacro()

## Creates a directory populated with all components related to the data partition
##   folder must end in "_data_partition"
## Optional argument can be used to specify dependency targets
## create_data_partition_directory(_EXECUTABLE_TARGET_NAME _FILES_TO_COPY _OPTIONAL_DEPENDS_TARGETS)
macro(create_data_partition_directory)
  if(${ARGC} EQUAL 2)
    add_custom_target(make_data_partition_${ARGV0} ALL
      COMMAND ${CMAKE_COMMAND} -E rm -rf ${ARGV0}_data_partition/
        COMMAND ${CMAKE_COMMAND} -E make_directory ${ARGV0}_data_partition/
        COMMAND ${CMAKE_COMMAND} -E copy ${ARGV1} ${ARGV0}_data_partition/
        COMMENT
            "Collect data partition components"
        VERBATIM
    )
  elseif(${ARGC} EQUAL 3)
    add_custom_target(make_data_partition_${ARGV0} ALL
      COMMAND ${CMAKE_COMMAND} -E rm -rf ${ARGV0}_data_partition/
      COMMAND ${CMAKE_COMMAND} -E make_directory ${ARGV0}_data_partition/
      COMMAND ${CMAKE_COMMAND} -E copy ${ARGV1} ${ARGV0}_data_partition/
      DEPENDS
          ${ARGV2}
      COMMENT
          "Collect data partition components"
      VERBATIM
    )
  else()
    message(FATAL_ERROR "Invalid number of arguments passed to create_data_partition_directory")
  endif()

  set_target_properties(make_data_partition_${ARGV0} PROPERTIES
      ADDITIONAL_CLEAN_FILES ${ARGV0}_data_partition
  )
endmacro()

## Creates a flash app target for a provided binary
## Optional arguments can be used to specify boot partition size, data partition contents, and other dependency targets, such as filesystem generators
## create_flash_app_target(_EXECUTABLE_TARGET_NAME _BOOT_PARTITION_SIZE _DATA_PARTITION_CONTENTS _OPTIONAL_DEPENDS_TARGETS)
function(create_flash_app_target)
  if(${ARGC} EQUAL 1)
    add_custom_target(flash_app_${ARGV0}
        COMMAND xflash --quad-spi-clock 50MHz --factory ${ARGV0}.xe
        DEPENDS ${ARGV0}
        COMMENT
          "Flash application"
        VERBATIM
      )
  elseif(${ARGC} EQUAL 2)
    add_custom_target(flash_app_${ARGV0}
        COMMAND xflash --quad-spi-clock 50MHz --factory ${ARGV0}.xe --boot-partition-size ${ARGV1}
        DEPENDS ${ARGV0}
        COMMENT
          "Flash application with empty data partition"
        VERBATIM
      )
  elseif(${ARGC} EQUAL 3)
    add_custom_target(flash_app_${ARGV0}
        COMMAND xflash --quad-spi-clock 50MHz --factory ${ARGV0}.xe --boot-partition-size ${ARGV1} --data ${ARGV2}
        DEPENDS ${ARGV0}
        COMMENT
          "Flash application and data partition"
        VERBATIM
      )
  elseif(${ARGC} EQUAL 4)
  add_custom_target(flash_app_${ARGV0}
        COMMAND xflash --quad-spi-clock 50MHz --factory ${ARGV0}.xe --boot-partition-size ${ARGV1} --data ${ARGV2}
        DEPENDS ${ARGV0} ${ARGV3}
        COMMENT
          "Flash application and data partition"
        VERBATIM
    )
  else()
    message(FATAL_ERROR "Invalid number of arguments passed to create_flash_app_target")
  endif()
endfunction()

## Creates an install target for a provided binary
macro(create_install_target _EXECUTABLE_TARGET_NAME)
    add_custom_target(install_${_EXECUTABLE_TARGET_NAME}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/dist
      COMMAND ${CMAKE_COMMAND} -E copy ${_EXECUTABLE_TARGET_NAME}.xe ${PROJECT_SOURCE_DIR}/dist
      DEPENDS ${_EXECUTABLE_TARGET_NAME}
      COMMENT
        "Install application"
    )
endmacro()

## Creates an xflash image upgrade target for a provided binary
macro(create_upgrade_img_target _EXECUTABLE_TARGET_NAME _FACTORY_MAJOR_VER _FACTORY_MINOR_VER)
    add_custom_target(create_upgrade_img_${_EXECUTABLE_TARGET_NAME}
      COMMAND xflash --factory-version ${_FACTORY_MAJOR_VER}.${_FACTORY_MINOR_VER} --upgrade 0 ${_EXECUTABLE_TARGET_NAME}.xe  -o ${_EXECUTABLE_TARGET_NAME}_upgrade.bin
      DEPENDS ${_EXECUTABLE_TARGET_NAME}
      COMMENT
        "Create upgrade image for application"
      VERBATIM
    )
endmacro()

## Creates an xflash erase all target for a provided target XN file
## Full filepath must be specified for XN file
macro(create_erase_all_target _APP_NAME _TARGET_FILEPATH)
    add_custom_target(erase_all_${_APP_NAME}
      COMMAND xflash --erase-all --target-file=${_TARGET_FILEPATH}
      DEPENDS
      COMMENT
        "Erase target flash"
      VERBATIM
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
    # Run cat "$XMOS_TOOL_PATH"/doc/version.txt
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E cat $ENV{XMOS_TOOL_PATH}/doc/version.txt
        OUTPUT_VARIABLE XCC_VERSION_OUTPUT_STRING
    )
    # Split output semver
    string(FIND ${XCC_VERSION_OUTPUT_STRING} " " SPACE_POSITION)
    string(SUBSTRING ${XCC_VERSION_OUTPUT_STRING} 0 ${SPACE_POSITION} XCC_SEMVER)
    # Parse version fields
    string(REPLACE "." ";" XCC_SEMVER_FIELDS ${XCC_SEMVER})
    list(LENGTH XCC_SEMVER_FIELDS XCC_SEMVER_FIELDS_LENGTH)
    list(GET XCC_SEMVER_FIELDS 0 XCC_VERSION_MAJOR)
    list(GET XCC_SEMVER_FIELDS 1 XCC_VERSION_MINOR)
    list(GET XCC_SEMVER_FIELDS 2 XCC_VERSION_PATCH)
    # Set XTC version env variables
    set(XTC_VERSION_MAJOR ${XCC_VERSION_MAJOR} PARENT_SCOPE)
    set(XTC_VERSION_MINOR ${XCC_VERSION_MINOR} PARENT_SCOPE)
    set(XTC_VERSION_PATCH ${XCC_VERSION_PATCH} PARENT_SCOPE)
endfunction()