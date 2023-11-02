#######################
Check RTOS Clock Driver
#######################

*******
Purpose
*******

Description
===========

The RTOS driver tests are designed to regression test RTOS driver behavior for the following drivers:

- clock driver

These tests assume that the associated RTOS and HILs used have been verified by their own localized separate testing.

These tests should be run whenever the code or submodules in ``modules\rtos`` or ``modules\hil`` are changed.

**************************
Building and Running Tests
**************************

To build the test application firmware, run the following command from the top of the repository:

.. code-block:: console

    bash tools/ci/build_rtos_tests.sh

The ``build_rtos_tests.sh`` script will copy the test applications to the ``dist`` folder.

Run the test with the following command from the top of the repository:

.. code-block:: console

    bash test/rtos_drivers/clock_control/check_clock_control.sh
