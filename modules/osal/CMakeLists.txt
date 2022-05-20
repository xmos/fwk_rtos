
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_osal_freertos INTERFACE)
    target_sources(framework_rtos_osal_freertos
        INTERFACE
            FreeRTOS/rtos_osal_event_group_port.c
            FreeRTOS/rtos_osal_heap.c
            FreeRTOS/rtos_osal_mutex_port.c
            FreeRTOS/rtos_osal_queue_port.c
            FreeRTOS/rtos_osal_semaphore_port.c
            FreeRTOS/rtos_osal_thread_port.c
            FreeRTOS/rtos_osal_time.c
    )
    target_include_directories(framework_rtos_osal_freertos
        INTERFACE
            api
            FreeRTOS
    )
    target_link_libraries(framework_rtos_osal_freertos
        INTERFACE
            rtos::rtos_support
            rtos::FreeRTOS::FreeRTOS_SMP
    )

    ## Create an alias
    add_library(rtos::osal ALIAS framework_rtos_osal_freertos)
endif()
