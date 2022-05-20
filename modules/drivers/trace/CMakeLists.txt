
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_drivers_trace INTERFACE)
    target_sources(framework_rtos_drivers_trace
        INTERFACE
            ASCII/ascii_trace.c
    )
    target_include_directories(framework_rtos_drivers_trace
        INTERFACE
            api
    )
    target_link_libraries(framework_rtos_drivers_trace
        INTERFACE
            rtos::FreeRTOS::FreeRTOS_SMP
    )

    ## Create an alias
    add_library(rtos::drivers::trace ALIAS framework_rtos_drivers_trace)
endif()
