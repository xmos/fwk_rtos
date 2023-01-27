#################
RTOS Driver Tests
#################

The RTOS driver tests are designed to regression test RTOS driver behavior for the following drivers:

- gpio
- i2c
- i2s
- intertile
- mic_array
- qspi_flash
- swmem

These tests assume that the associated RTOS and HILs used have been verified by their own localized separate testing.

These tests should be run whenever the local driver code or dependencies in ``fwk_core`` or ``fwk_io`` are changed.

**************
Hardware Setup
**************

The target hardware for these tests is the XCORE-AI-EXPLORER board.

To setup the board for testing, the following connections must be made:

============  ================  ================  =====================================
Pin Desc hil  Pin Desc hil_add  Connection        Port Name
============  ================  ================  =====================================
GPIO I/O      Unused            X1D02 : X1D39     T1 XS1_PORT_4A Pin 0 : T1 XS1_PORT_1P
I2C SCL       UART TX/RX        SCL IOL : X1D36   T0 XS1_PORT_1N : T1 XS1_PORT_1M
I2C SDA       SPI CS            SDA IOL : X1D38   T0 XS1_PORT_1O : T1 XS1_PORT_1O
I2S DACD      SPI MOSI          DAC_DAT : X0D12   T1 XS1_PORT_1A : T0 XS1_PORT_1E
I2S ADCD      SPI MISO          ADC_DAT : X0D13   T1 XS1_PORT_1N : T0 XS1_PORT_1F
I2S BCLK      Unused            BCLK : X0D22      T1 XS1_PORT_1C : T0 XS1_PORT_1G
I2S LRCLK     SPI SCLK          LRCLK : X0D23     T1 XS1_PORT_1B : T0 XS1_PORT_1H
============  ================  ================  =====================================

Wiring Diagram
==============

.. image:: images/wiring_diagram.png
    :align: left

xsim --plugin LoopbackPort.dll "-port tile[1] XS1_PORT_4A 1 0 -port tile[1] XS1_PORT_1P 1 0" --plugin LoopbackPort.dll "-port tile[0] XS1_PORT_1N 1 0 -port tile[1] XS1_PORT_1M 1 0" --plugin LoopbackPort.dll "-port tile[0] XS1_PORT_1O 1 0 -port tile[1] XS1_PORT_1O 1 0" --plugin LoopbackPort.dll "-port tile[1] XS1_PORT_1A 1 0 -port tile[0] XS1_PORT_1E 1 0" --plugin LoopbackPort.dll "-port tile[1] XS1_PORT_1N 1 0 -port tile[0] XS1_PORT_1F 1 0" --plugin LoopbackPort.dll "-port tile[1] XS1_PORT_1C 1 0 -port tile[0] XS1_PORT_1G 1 0" --plugin LoopbackPort.dll "-port tile[1] XS1_PORT_1B 1 0 -port tile[0] XS1_PORT_1H 1 0" --xscope "-offline trace.xmt" test_rtos_driver_hil_add.xe 
