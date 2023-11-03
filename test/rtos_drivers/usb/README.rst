#####################
RTOS Driver USB Tests
#####################

*******
Purpose
*******

Description
===========

This test is designed to perform basic regression tests of the RTOS USB driver.

This test assumes that the associated RTOS and HILs used have been verified by
their own localized separate testing and that the rtos_drivers tests are passing.

This test should be run whenever the code or submodules in ``modules\rtos`` or ``modules\hil`` are changed.

**************
Hardware Setup
**************

The target hardware for these tests is the XCORE-AI-EXPLORER board.

**********
Host Setup
**********

These tests require the `dfu-util <https://dfu-util.sourceforge.net/>`_ application,
which is available in binary and source form and may be available on select
package management systems.

Example for Debian:

.. code-block:: console

    $ apt-get install dfu-util

On Windows platforms, prior to running the test, the user must manually update
the driver for the enumerated DFU interface using a driver utility such as
`Zadig <https://zadig.akeo.ie/>`_ to use one of the available ``libusb``
compatible drivers.

**************
Building Tests
**************

To build this test among other tests that are part of the CI test framework,
run the following command from the root of this repository:

.. code-block:: console

    bash tools/ci/build_rtos_tests.sh

The ``build_rtos_tests.sh`` script will copy the test applications to the ``dist`` folder.

*************
Running Tests
*************

With the test application built, the test may be executed by running the
following command from the root of this repository:

.. code-block:: console

    $ ./test/rtos_drivers/usb/check_usb.sh

The target's test application outputs directly to the console along with the
various test steps being performed. On error/failure, the host's logged results
will also be emitted to the console. Logs of the target's test application
output and host's test output are stored separately in the ``./testing`` folder.


The output file can be verified via python:

.. code-block:: console

    pytest test/rtos_drivers/usb
