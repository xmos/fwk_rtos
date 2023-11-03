
.. _build_system_guide:

############
Build System
############

This document describes the `CMake <https://cmake.org/>`_-based build system used by applications based on the XMOS RTOS framework.  The build system is designed so a user does not have to be an expert using CMake.  However, some familiarity with CMake is helpful.  You can familiarize yourself by reading the `CMake Tutorial <https://cmake.org/cmake/help/latest/guide/tutorial/index.html>`_ or `CMake documentation <https://cmake.org/cmake/help/v3.20/>`_.  Reviewing these is optional and the reader should feel free to save that for later.  

********
Overview
********

An xcore RTOS project can be seen as an integration of several modules. For example, for a FreeRTOS application that captures audio from PDM microphones and outputs it to a DAC, there could be the following modules:

- Several core modules (for debug prints, etc...)
- The FreeRTOS kernel and drivers
- PDM microphone array driver for receiving audio samples
- |I2C| driver for configuring the DAC
- |I2S| driver for outputting to the DAC
- Application code tying it all together

When a project is compiled, the build system will build all libraries and source files required for the application. For this to happen, your ``CMakeLists.txt`` file will need to specify:

- Application sources and include paths
- Compile flags
- Compile definitions
- Link libraries
- Link options

This is best illustrated with a commented :ref:`build_system_example_cmakelists`.

*******
Aliases
*******

Your `CMakeLists.txt` file will need to specify the `target link libraries <https://cmake.org/cmake/help/latest/command/target_link_libraries.html>`_ as shown in the following snippet:

.. code-block:: cmake

    target_link_libraries(my_target PUBLIC 
        core::general
        rtos::freertos
        rtos::drivers::mic_array
        rtos::drivers::i2c
        rtos::drivers::i2s
        lib_mic_array
        lib_i2c
        lib_i2s
    )

It is very common for target link `alias libraries <https://cmake.org/cmake/help/latest/command/add_library.html#alias-libraries>`_, like ``rtos::freertos`` in the snippet above, to include common sets of target link libraries. The snippet above could be simplified because the `rtos::freertos` alias includes many commonly used drivers and peripheral IO libraries as a dependency.

.. code-block:: cmake

    target_link_libraries(my_target PUBLIC 
        core::general
        rtos::freertos
    )

Application target link libraries can be further simplified using existing bsp_configs. These provide their dependent link libraries enabling applications to simplify their target link libraries list. The snippet above could be simplified because the `rtos::bsp_config::xcore_ai_explorer` alias includes `core::general`, `rtos::freertos`, and all required drivers and peripheral IO libraries used by the bsp_config. More information on bsp_configs can be found in the RTOS Programming Guide.

.. code-block:: cmake

    target_link_libraries(my_target PUBLIC 
        rtos::bsp_config::xcore_ai_explorer
    )

XMOS libraries and frameworks provide several target aliases. Being aware of the :ref:`build_system_targets` will simplify your application ``CMakeLists.txt``.
