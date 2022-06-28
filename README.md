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
- Job dispatcher

This repository can be used standalone.  However, it is meant to be used as part of the [XCORE SDK](https://github.com/xmos/xcore_sdk).

## Build Status

Build Type       |    Status     |
-----------      | --------------|
Docs             | ![CI](https://github.com/xmos/fwk_rtos/actions/workflows/docs.yml/badge.svg?branch=develop&event=push) |

## Cloning

Some dependent components are included as git submodules. These can be obtained by cloning this repository with the following command:

    $ git clone --recurse-submodules https://github.com/xmos/fwk_rtos.git

## Testing

No tests exist in this repository yet.  Several tests for the RTOS framework modules exist in the [XCORE SDK tests](https://github.com/xmos/xcore_sdk/tree/develop/test).  

## Documentation

Information on building the documentation can be found in the docs [README](https://github.com/xmos/fwk_rtos/blob/develop/doc/README.rst).

## License

This Software is subject to the terms of the [XMOS Public Licence: Version 1](https://github.com/xmos/fwk_rtos/blob/develop/LICENSE.rst)

Third party copyrighted code is specified in the XCORE SDK [Copyrights and Licenses](https://github.com/xmos/xcore_sdk/blob/develop/doc/copyright.rst).  

