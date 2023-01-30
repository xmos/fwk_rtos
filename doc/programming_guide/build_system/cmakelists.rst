######################
Example CMakeLists.txt
######################

.. note::

   CMake is powerful tool that provides the developer a great deal of flexibility in how their projects are built.  As a result, CMakeLists.txt files in the example applications may vary from the examples below.  This example can be used as a starting point for your application.  Or, you may choose to copy a ``CMakeLists.txt`` from a reference design or other example application that closely resembles your application.

Below is an example ``CMakeLists.txt`` that shows both required and conventional commands for a basic FreeRTOS project.  This example can be used as a starting point for your FreeRTOS application.  Or, you may choose to copy a ``CMakeLists.txt`` from one of the applications in the SDK that closely resembles your application.

.. code-block:: cmake

   ## Specify your application sources and includes
   file(GLOB_RECURSE APP_SOURCES   src/*.c )
   set(APP_INCLUDES src src/platform)

   ## Specify your compiler flags
   set(APP_COMPILER_FLAGS
      -Os
      -report
      -fxscope
      -mcmodel=large
      ${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope
      ${CMAKE_CURRENT_SOURCE_DIR}/XCORE-AI-EXPLORER.xn
   )

   ## Specify any compile definitions
   set(APP_COMPILE_DEFINITIONS
      DEBUG_PRINT_ENABLE=1
      PLATFORM_SUPPORTS_TILE_0=1
      PLATFORM_SUPPORTS_TILE_1=1
      PLATFORM_SUPPORTS_TILE_2=0
      PLATFORM_SUPPORTS_TILE_3=0
      PLATFORM_USES_TILE_0=1
      PLATFORM_USES_TILE_1=1
   )

   ## Set your link options
   set(APP_LINK_OPTIONS
      -report
      ${CMAKE_CURRENT_SOURCE_DIR}/XCORE-AI-EXPLORER.xn
      ${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope
   )

   ## Create your targets
   set(TARGET_NAME tile0_my_app)
   add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
   target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
   target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
   target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
   target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
   target_link_libraries(${TARGET_NAME} PUBLIC core::general rtos::freertos)
   target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
   unset(TARGET_NAME)

   set(TARGET_NAME tile1_my_app)
   add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
   target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
   target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
   target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
   target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
   target_link_libraries(${TARGET_NAME} PUBLIC core::general rtos::freertos)
   target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} )
   unset(TARGET_NAME)

   ## Merge tile0 and tile1 binaries
   merge_binaries(my_app tile0_my_app tile1_my_app 1)

   ## Optionally create run and debug targets
   create_run_target(my_app)
   create_debug_target(my_app)
