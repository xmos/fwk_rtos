.. _build_system_guide_macros:

******
Macros
******

Several CMake macros and functions are provide to make building for XCORE easier.  These macros are located in the file `tools/cmake_utils/xmos_macros.cmake <https://github.com/xmos/fwk_rtos/blob/develop/tools/cmake_utils/xmos_macros.cmake>`_ and are documented below.  

To see what XTC Tools commands the macros and functions are running, add ``VERBOSE=1`` to your build command line.  For example:

.. code-block:: console

    make run_my_target VERBOSE=1

Common Macros
=============

merge_binaries
--------------

merge_binaries combines multiple xcore applications into one by extracting a tile elf and recombining it into another binary. This is used in multitile RTOS applications to enable building unique instances of the FreeRTOS kernel and task sets on a per tile basis.
This macro takes an output target name, a base target, a target containing a tile to merge, and the tile number to merge.

This macro can be called in two ways. The 4 argument version is for when the
application has only 1 node and therefore only the core needs to be specified.

.. code-block:: cmake

   # create target OUT by replacing tile number 0 in BASE with tile 0 in OTHER
   merge_binaries(${OUT} ${BASE} ${OTHER} 0)

The 5 argument version is for multi-node applications. IMPORTANT: node number 
is not the "Node Id" from the xn file, rather the index of the node in the 
JTAGChain which is defined in the xn file.

.. code-block:: cmake

   # create target OUT by replacing tile 1 on node 0 in BASE with tile 1 on 
   # node 0 in OTHER
   merge_binaries(${OUT} ${BASE} ${OTHER} 0 1)

create_run_target
-----------------

create_run_target creates a run target for ``<TARGET_NAME>`` with xscope output.  

.. code-block:: cmake

  create_run_target(<TARGET_NAME>)

create_run_target allows you to run a binary with the following command instead of invoking ``xrun --xscope``.

.. code-block:: console

    make run_my_target


create_debug_target
-------------------

create_debug_target creates a debug target for ``<TARGET_NAME>``.  

.. code-block:: cmake

  create_debug_target(<TARGET_NAME>)

create_debug_target allows you to debug a binary with the following command instead of invoking ``xgdb``.  This target implicitly sets up the xscope debug interface as well.

.. code-block:: console

    make debug_my_target


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



create_flash_app_target
-----------------------

create_flash_app_target creates a debug target for ``<TARGET_NAME>`` with optional arguments ``<BOOT_PARTITION_SIZE>``, ``<DATA_PARTITION_CONTENTS>``, and ``<OPTIONAL_DEPENDS_TARGETS>``. ``<BOOT_PARTITION_SIZE>`` specificies the size in bytes of the boot partition. ``<DATA_PARTITION_CONTENTS>`` specifies the optional binary contents of the data partition. ``<OPTIONAL_DEPENDS_TARGETS>`` specifies CMake targets that should be dependencies of the resulting create_flash_app_target target. This may be used to create recipes that generate the data partition contents.

.. code-block:: cmake

  create_flash_app_target(<TARGET_NAME> <BOOT_PARTITION_SIZE> <DATA_PARTITION_CONTENTS> <OPTIONAL_DEPENDS_TARGETS>)

create_flash_app_target allows you to flash a factory image binary and optional data partition with the following command instead of invoking ``xflash``.

.. code-block:: console

    make flash_app_my_target


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

create_upgrade_img_target creates an xflash image upgrade target for a provided binary for use in DFU

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
