#########################
Clock Control RTOS Driver
#########################

This driver can be used to operate GPIO ports on xcore in an RTOS application.

******************
Initialization API
******************
The following structures and functions are used to initialize and start a GPIO driver instance.

.. doxygengroup:: rtos_clock_control_driver
   :content-only:

********
Core API
********

The following functions are the core GPIO driver functions that are used after it has been initialized and started.

.. doxygengroup:: rtos_clock_control_driver_core
   :content-only:

**********************
RPC Initialization API
**********************

The following functions may be used to share a GPIO driver instance with other xcore tiles. Tiles that the
driver instance is shared with may call any of the core functions listed above.

.. doxygengroup:: rtos_clock_control_driver_rpc
   :content-only:
