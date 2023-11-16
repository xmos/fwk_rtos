######################
RTOS Driver WiFi Tests
######################

*******
Purpose
*******

Description
===========

This test is designed to perform basic regression tests of the RTOS WIFI driver.

This test assume that the associated RTOS and HILs used have been verified by their own localized separate testing and that the rtos_drivers tests are passing.

This test should be run whenever the code or submodules in ``modules\rtos`` or ``modules\hil`` are changed.

**************
Hardware Setup
**************

The target hardware for these tests is the XCORE-AI-EXPLORER board.

**************************
Building and Running Tests
**************************

.. note::

    The Python environment is required to run this test.  See the Requirements section of test/README.rst

To build the test host applications, run the following command from the top of the repository:

.. code-block:: console

    bash tools/ci/build_host_apps.sh

The ``build_host_apps.sh`` script will copy the test applications to the ``dist_host`` folder.


To build the test application firmware, run the following command from the top of the repository:

.. code-block:: console

    bash tools/ci/build_rtos_tests.sh

The ``build_rtos_tests.sh`` script will copy the test applications to the ``dist`` folder.

Run the test with the following command from the top of the repository:

.. code-block:: console

    bash test/rtos_drivers/wifi/check_wifi.sh


The output file can be verified via python:

.. code-block:: console

    pytest test/rtos_drivers/wifi
