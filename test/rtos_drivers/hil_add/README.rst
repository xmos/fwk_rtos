############################
RTOS Driver Additional Tests
############################

The RTOS driver tests are designed to regression test RTOS driver behavior for the following drivers:

- spi
- uart
- qspi_fast_read

These tests assume that the associated RTOS and HILs used have been verified by their own localized separate testing.

These tests should be run whenever the local driver code or dependencies in ``fwk_core`` or ``fwk_io`` are changed.

**************
Hardware Setup
**************

The target hardware for these tests is the XCORE-AI-EXPLORER board.

To setup the board for testing, refer to the Hardware Setup in `RTOS Driver HIL Tests <https://github.com/xmos/fwk_rtos/blob/develop/test/rtos_drivers/hil/README.rst>`_

Additionally, the flash must contain the lib_qspi_fast_read calibration image.  This can be flashed via xflash commands or by making the custom cmake target ``flash_calibration_test_rtos_driver_hil_add``.