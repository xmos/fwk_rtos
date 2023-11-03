
############################
Board Support Configurations
############################

xcore leverages its architecture to provide a flexible chip where many typically silicon based peripherals are found in software. This allows a chip to be reconfigured in a way that provides the specific IO required for a given application, thus resulting in a low cost yet incredibly silicon efficient solution. Board support configurations (bsp_configs) are the description for the hardware IO that exists in a given board. The bsp_configs provide the application programmer with an API to initialize and start the hardware configuration, as well as the supported RTOS driver contexts. The programming model in this FreeRTOS architecture is:

- `.xn files <https://www.xmos.ai/documentation/XM-014363-PC-LATEST/html/tools-guide/tools-ref/formats/xn-spec/xn-spec.html?highlight=xn>`_ provide the mapping of ports, pins, and links
- bsp_configs specify, setup, and start hardware IO and provide the application with RTOS driver contexts
- applications use the bsp_config init/start code as well as RTOS driver contexts, similar to conventional microcontroller programming models.

To support any generic bsp_config, applications should call ``platform_init()`` before starting the scheduler, and then `platform_start()` after the scheduler is running and before any RTOS drivers are used.

The bsp_configs provided with the RTOS framework in `modules/rtos/modules/bsp_config <https://github.com/xmos/fwk_rtos/tree/develop/modules/bsp_config>`_ are an excellent starting point. They provide the most common peripheral drivers that are supported by the boards that support RTOS framework based applications. For advanced users, it is recommended that you copy one of these bsp_config into your application project and customize as needed.

***************************
Creating Custom bsp_configs
***************************

To enable hardware portability, a minimal ``bsp_config`` should contain the following:

.. code-block:: console

  custom_config/
    platform/
      driver_instances.c
      driver_instances.h
      platform_conf.h
      platform_init.c
      platform_init.h
      platform_start.c
    custom_config.cmake
    custom_config_xn_file.xn


``custom_config.cmake`` provides the ``CMake`` target of the configuration.  This target should link the required RTOS framework libraries to support the configuration it defines.

``custom_config_xn_file.xn`` provides various hardware parameters including but not limited to the chip package, IO mapping, and network information.

``platform_conf.h`` provides default configuration of all header defined configuration macros. These may be overridden by compile definitions or application headers.

``driver_instances.h`` provides the declaration of all RTOS drivers in the configuration. It may define XCORE hardware resources, such as ports and clockblocks. It may also define tile placements.

``driver_instances.c`` provides the definition of all RTOS drivers in the configuration.

``platform_init.h`` provides the declaration of ``platform_init(chanend_t other_tile_c)`` and ``platform_start(void)``

``platform_init.c`` provides the initialization of all drivers defined in the configuration through the definition of ``platform_init(chanend_t other_tile_c)``. This code is run before the scheduler is started and therefore will not be able to access all RTOS driver functionalities nor kernel objects.

``platform_start.c`` provides the starting of all drivers defined in the configuration through the definition of ``platform_start(void)``. It may also perform any initialization setup, such as configuring the app_pll or setting up an on board DAC. This code is run once the kernel is running and is therefore subject to preemption and other dynamic scheduling SMP programming considerations.
