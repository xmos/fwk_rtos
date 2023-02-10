
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_drivers_clock_control INTERFACE)
    target_sources(framework_rtos_drivers_clock_control
        INTERFACE
            src/rtos_clock_control.c
            src/rtos_clock_control_rpc.c
    )
    target_include_directories(framework_rtos_drivers_clock_control
        INTERFACE
            api
    )
    target_link_libraries(framework_rtos_drivers_clock_control
        INTERFACE
            framework_core_clock_control
            rtos::osal
            rtos::sw_services::concurrency_support
    )

    ## Create an alias
    add_library(rtos::drivers::clock_control ALIAS framework_rtos_drivers_clock_control)
endif()
