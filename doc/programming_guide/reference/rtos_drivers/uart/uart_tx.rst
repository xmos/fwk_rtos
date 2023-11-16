   
###################
UART Tx RTOS Driver
###################

This driver can be used to instantiate and control an UART Tx I/O interface on xCORE in an RTOS application.

***********
UART Tx API
***********

The following structures and functions are used to initialize and start a UART Tx driver instance.

.. doxygengroup:: rtos_uart_tx_driver
   :content-only:

******************************
UART Tx RPC Initialization API
******************************

The following functions may be used to share a UART Tx driver instance with other xCORE tiles. Tiles that the
driver instance is shared with may call any of the core functions listed above.

.. doxygengroup:: rtos_uart_tx_driver_rpc
   :content-only:

