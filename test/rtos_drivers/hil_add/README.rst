###################################
Check RTOS Drivers Additional Tests
###################################

*******
Purpose
*******

Description
===========

The RTOS driver tests are designed to regression test RTOS driver behavior for the following drivers:

- spi
- qspi_fast_read

These tests assume that the associated RTOS and HILs used have been verified by their own localized separate testing.

These tests should be run whenever the local driver code or dependencies in ``fwk_core`` or ``fwk_io`` are changed.

**************
Hardware Setup
**************

The target hardware for these tests is the XCORE-AI-EXPLORER board.

To setup the board for testing, refer to the Hardware Setup in `RTOS Driver HIL Tests <https://github.com/xmos/fwk_rtos/blob/develop/test/rtos_drivers/hil/README.rst>`_

Additionally, the flash must contain the lib_qspi_fast_read calibration image.  This can be flashed via xflash commands or by making the custom cmake target ``flash_calibration_test_rtos_driver_hil_add``.

**************************
Building and Running Tests
**************************

.. note::

    The Python environment is required to run this test.  See the Requirements section of test/README.rst

To build the test application firmware, run the following command from the top of the repository:

.. code-block:: console

    bash tools/ci/build_rtos_tests.sh

The ``build_rtos_tests.sh`` script will copy the test applications to the ``dist`` folder.

Run the test with the following command from the top of the repository:

.. code-block:: console

    bash test/rtos_drivers/hil_add/check_drivers_hil_add.sh


The output file can be verified via python:

.. code-block:: console

    pytest test/rtos_drivers/hil_add
