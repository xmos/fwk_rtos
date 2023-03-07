##################################
Device Firmware Update RTOS Driver
##################################

This driver can be used to instantiate and manipulate various flash partitions on xcore in an RTOS application.

For application usage refer to the tutorial :doc:`RTOS Application DFU <../../tutorials/application_dfu_usage>`.

******************
Initialization API
******************
The following structures and functions are used to initialize and start a DFU driver instance.

.. doxygengroup:: rtos_dfu_image_driver
   :content-only:

********
Core API
********

The following functions are the core DFU driver functions that are used after it has been initialized and started.

.. doxygengroup:: rtos_dfu_image_driver_core
   :content-only:
