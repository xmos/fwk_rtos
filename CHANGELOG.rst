RTOS Framework change log
=========================

3.0.5
-----

  * FIXED: Added dummy definitions of vTaskCoreAffinitySet() and vTaskCoreAffinitySet() functions to
    avoid warnings when configNUM_CORES is set to 1.
  * UPDATED: To fwk_io v3.3.0 from v3.0.1 for CI run
  * UPDATED: To fwk_core v1.0.2 from v1.0.0 for CI run
  * UPDATED: Use tagged release v1.0.1 of lib_qspi_fast_read instead of 85fe541 hash

3.0.4
-----

  * FIXED: Issue seen with EP0 becoming unresponsive when lots of volume control and EP0 vendor specific
    control commands are issued at the same time.

3.0.3
-----
  
  * FIXED: Now fully supports overriding implementations for FatFS IO functions.

3.0.2
-----
  
  * UPDATED: To test again fwk_io v3.0.1
  * UPDATED: To test again lib_qspi_fast_read v1.0.1
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
