
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_rtos_support INTERFACE)
    target_sources(framework_rtos_rtos_support
        INTERFACE
            src/rtos_cores.c
            src/rtos_irq.c
            src/rtos_locks.c
            src/rtos_time.c
    )
    target_include_directories(framework_rtos_rtos_support
        INTERFACE
            api
            src
    )
    target_link_libraries(framework_rtos_rtos_support
        INTERFACE
            rtos::FreeRTOS::FreeRTOS_SMP
    )

    ## Create an alias
    add_library(rtos::rtos_support ALIAS framework_rtos_rtos_support)
endif()
