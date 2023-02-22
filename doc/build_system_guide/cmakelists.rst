.. _build_system_example_cmakelists:

######################
Example CMakeLists.txt
######################

CMake is powerful tool that provides the developer a great deal of flexibility in how their projects are built.  As a result, ``CMakeLists.txt`` files can accomplish the same function in multiple ways.

Below is an example ``CMakeLists.txt`` that shows both required and conventional commands for a basic FreeRTOS project.  This example can be used as a starting point for your application, but it is recommended to copy a ``CMakeLists.txt`` from an XMOS reference design or other example application that closely resembles your application.

.. code-block:: cmake

   ## Specify your application sources by globbing the src folder
   file(GLOB_RECURSE APP_SOURCES src/*.c)

   ## Specify your application include paths
   set(APP_INCLUDES src)

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
      configENABLE_DEBUG_PRINTF=1
      PLATFORM_USES_TILE_0=1
      PLATFORM_USES_TILE_1=1
   )

   ## Set your link libraries
   set(APP_LINK_LIBRARIES
      rtos::bsp_config::xcore_ai_explorer
   )

   ## Set your link options
   set(APP_LINK_OPTIONS
      -report
      ${CMAKE_CURRENT_SOURCE_DIR}/XCORE-AI-EXPLORER.xn
      ${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope
   )

   ## Create your targets

   ## Create the target for the portion of application code that will execute on tile[0] 
   set(TARGET_NAME tile0_my_app)
   add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
   target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
   target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
   target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
   target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
   target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
   target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
   unset(TARGET_NAME)

   ## Create the target for the portion of application code that will execute on tile[1] 
   set(TARGET_NAME tile1_my_app)
   add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
   target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
   target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
   target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
   target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
   target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
   target_link_libraries(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} )
   unset(TARGET_NAME)

   ## Merge tile[0] and tile[1] binaries into a single binary using an XMOS CMake macro
   merge_binaries(my_app tile0_my_app tile1_my_app 1)

   ## Optionally create run and debug targets using XMOS CMake macros
   create_run_target(my_app)
   create_debug_target(my_app)

For more information, see the documentation for each of the `CMake commands <https://cmake.org/cmake/help/latest/manual/cmake-commands.7.html>`_ used in the example above.

- `set <https://cmake.org/cmake/help/latest/command/set.html>`_
- `add_executable <https://cmake.org/cmake/help/latest/command/add_executable.html>`_
- `target_sources <https://cmake.org/cmake/help/latest/command/target_sources.html>`_
- `target_include_directories <https://cmake.org/cmake/help/latest/command/target_include_directories.html>`_
- `target_compile_definitions <https://cmake.org/cmake/help/latest/command/target_compile_definitions.html>`_
- `target_compile_options <https://cmake.org/cmake/help/latest/command/target_compile_options.html>`_
- `target_link_libraries <https://cmake.org/cmake/help/latest/command/target_link_libraries.html>`_
- `target_link_options <https://cmake.org/cmake/help/latest/command/target_link_options.html>`_

See :ref:`build_system_guide_macros` for more information on the XMOS CMake macros.  