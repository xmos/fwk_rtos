RTOS Framework change log
=========================

3.0.2
-----
  
  * ADDED: Retry connect behavior to lib_quadflash portion of RTOS flash driver
  * FIXED: Fixed device control host for Windows.

3.0.1
-----
  
  * FIXED: Fix to stop dest_ctrl_buffer being overwritten when xfers on EPs other than EP0 are initiated

3.0.0
-----
  
  * REMOVED: QSPI IO based RTOS flash driver
  * ADDED: lib_qspi_fast_read and lib_quadflash based RTOS flash driver
  * ADDED: RTOS flash driver function to set core affinity for QSPI transactions
  * ADDED: New API to specify raw or nibble swapped individual QSPI flash reads
  * UPDATED: Mbed TLS to version 2.28.3

2.0.0
-----
  
  * CHANGE: USB driver now supports XUD v2.2.2

1.1.0
-----
  
  * ADDED: Improved SPI device control support

1.0.0
-----

  * Initial version
