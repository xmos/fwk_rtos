# RTOS Framework Repository

This repository is a collection of C/C++ RTOS libraries used to develop for xcore:

Base libraries include:

- FreeRTOS SMP kernel
- Board support package configurations
- Operating system abstraction layer (OSAL)
- Intertile driver support

Supported peripheral RTOS drivers include:

- GPIO
- UART
- I2C
- I2S
- SPI
- QSPI flash
- PDM microphones
- USB

Additional RTOS drivers include:

- Clock control
- L2 cache
- SwMem
- WiFi

Supported RTOS stacks and software services include:

- TinyUSB
- Generic processing pipeline
- Inferencing
- Device control
- FatFS
- HTTP
- TLS
- DHCP
- JSON
- MQTT
- WiFi

## Build Status

Build Type       |    Status     |
-----------      | --------------|
Docs             | ![CI](https://github.com/xmos/fwk_rtos/actions/workflows/docs.yml/badge.svg?branch=develop&event=push) |

## Cloning

Some dependent components are included as git submodules. These can be obtained by cloning this repository with the following command:

    $ git clone --recurse-submodules https://github.com/xmos/fwk_rtos.git

## Testing

Several tests for the RTOS framework modules exist in the [test folder](https://github.com/xmos/fwk_rtos/tree/develop/test).

## Documentation

This folder contains source files for the documentation and is intended for XMOS users. Pre-built documentation is published on https://www.xmos.com.

The sources do not render well in GitHub or an RST viewer.

## License

This Software is subject to the terms of the [XMOS Public Licence: Version 1](https://github.com/xmos/fwk_rtos/blob/develop/LICENSE.rst)

Third party copyrighted code is specified in the fwk_rtos [Copyrights and Licenses](https://github.com/xmos/fwk_rtos/blob/develop/doc/copyright.rst).

