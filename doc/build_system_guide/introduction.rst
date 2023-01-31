.. include:: ../substitutions.rst

.. _build_system_guide:

############
Build System
############

This document describes the `CMake <https://cmake.org/>`_-based build system used by the applications based on the RTOS framework.  The build system is designed so a user does not have to be an expert using CMake.  However, some familiarity with CMake is helpful.  You can familiarize yourself by reading the `Cmake Tutorial <https://cmake.org/cmake/help/latest/guide/tutorial/index.html>`_ or `CMake documentation <https://cmake.org/cmake/help/v3.20/>`_.

********
Overview
********

An xcore RTOS project can be seen as an integration of several modules. For example, for a FreeRTOS application that captures audio from PDM microphones and outputs it to a DAC, there could be the following modules:

- The SDK core modules (for debug prints, etc...)
- The FreeRTOS kernel
- Microphone array driver for audio samples
- |I2C| driver for configuring the DAC
- |I2S| driver for output to the DAC
- Application code tying it all together

When a project is compiled, the build system will build all libraries and source files specified by the application.  
