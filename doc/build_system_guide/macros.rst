.. _build_system_guide_macros:

******
Macros
******

Several CMake macros and functions are provide to make building for XCORE easier.  These macros are located in the file `tools/cmake_utils/xmos_macros.cmake <https://github.com/xmos/fwk_rtos/blob/develop/tools/cmake_utils/xmos_macros.cmake>`_ and are documented below.  

Common Macros
=============

merge_binaries
--------------

merge_binaries combines multiple xcore applications into one by extracting a tile elf and recombining it into another binary.
This macro takes an output target name, a base target, a target containing a tile to merge, and the tile number to merge.
The resulting output will be a target named ``<OUTPUT_TARGET_NAME>``, which contains the ``<BASE_TARGET>`` application with tile ``<TILE_NUM_TO_MERGE>`` replaced with
the respective tile from ``<OTHER_TARGET>``.

.. code-block:: cmake

  merge_binaries(<OUTPUT_TARGET_NAME> <BASE_TARGET> <OTHER_TARGET> <TILE_NUM_TO_MERGE>)


create_run_target
-----------------

create_run_target creates a run target for ``<TARGET_NAME>``.  

.. code-block:: cmake

  create_run_target(<TARGET_NAME>)

create_run_target allows you to run a binary with the following command instead of invoking ``xrun``.

.. code-block:: console

    make run_my_target


create_debug_target
-------------------

create_debug_target creates a debug target for ``<TARGET_NAME>``.  

.. code-block:: cmake

  create_debug_target(<TARGET_NAME>)

create_debug_target allows you to debug a binary with the following command instead of invoking ``xgdb``.

.. code-block:: console

    make debug_my_target


create_flash_app_target
-----------------------

create_flash_app_target creates a debug target for ``<TARGET_NAME>``.  

.. code-block:: cmake

  create_flash_app_target(<TARGET_NAME>)

create_flash_app_target allows you to flash binary with the following command instead of invoking ``xflash``.

.. code-block:: console

    make flash_my_target


create_filesystem_target
------------------------

create_filesystem_target creates a filesystem file for ``<TARGET_NAME>`` using the files in the ``<FILESYSTEM_INPUT_DIR>`` directory.  ``<IMAGE_SIZE>`` specifies the size (in bytes) of the filesystem.  The filesystem output filename will end in ``_fat.fs``.  Optional argument ``<OPTIONAL_DEPENDS_TARGETS>`` can be used to specify other dependency targets, such as filesystem generators.

.. code-block:: cmake

  create_filesystem_target(<TARGET_NAME> <FILESYSTEM_INPUT_DIR> <IMAGE_SIZE> <OPTIONAL_DEPENDS_TARGETS>)


create_data_partition_directory
-------------------------------

create_data_partition_directory creates a directory populated with all components related to the data partition. The data partition output folder will end in ``_data_partition``
Optional argument ``<OPTIONAL_DEPENDS_TARGETS>`` can be used to specify other dependency targets.

.. code-block:: cmake

  create_data_partition_directory(<TARGET_NAME> <FILES_TO_COPY> <OPTIONAL_DEPENDS_TARGETS>)


Less Common Macros
==================


create_install_target
---------------------

create_install_target creates an install target for ``<TARGET_NAME>``.

.. code-block:: cmake

  create_install_target(<TARGET_NAME>)

create_install_target will copy ``<TARGET_NAME>.xe`` to the ``${PROJECT_SOURCE_DIR}/dist`` directory.

.. code-block:: console

    make install_my_target


create_run_xscope_to_file_target
--------------------------------

create_run_xscope_to_file_target creates a run target for ``<TARGET_NAME>``. ``<XSCOPE_FILE>`` specifies the file to save to (no extension).

.. code-block:: cmake

  create_run_xscope_to_file_target(<TARGET_NAME> <XSCOPE_FILE>)

create_run_xscope_to_file_target allows you to run a binary with the following command instead of invoking ``xrun --xscope-file``.

.. code-block:: console

    make run_xscope_to_file_my_target


create_upgrade_img_target
-------------------------

create_upgrade_img_target creates an xflash image upgrade target for a provided binary

.. code-block:: cmake

  create_data_partition_directory(<TARGET_NAME> <FACTORY_MAJOR_VER> <FACTORY_MINOR_VER>)


create_erase_all_target
-----------------------

create_erase_all_target creates an xflash erase all target for ``<TARGET_FILEPATH>`` target XN file.  The full filepath must be specified for XN file

.. code-block:: cmake

  create_filesystem_target(<TARGET_NAME> <TARGET_FILEPATH>)

create_erase_all_target allows you to erase flash with the following command instead of invoking ``xflash``.

.. code-block:: console

    make erase_all_my_target


query_tools_version
-------------------

query_tools_version populates the following CMake variables:

    ``XTC_VERSION_MAJOR``
    ``XTC_VERSION_MINOR``
    ``XTC_VERSION_PATCH``

.. code-block:: cmake

    query_tools_version()