
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_sw_services_concurrency_support INTERFACE)
    target_sources(framework_rtos_sw_services_concurrency_support
        INTERFACE
            src/mrsw_lock.c
    )
    target_include_directories(framework_rtos_sw_services_concurrency_support
        INTERFACE
            api
    )
    target_link_libraries(framework_rtos_sw_services_concurrency_support
        INTERFACE
            rtos::osal
    )

    ## Create an alias
    add_library(rtos::sw_services::concurrency_support ALIAS framework_rtos_sw_services_concurrency_support)
endif()
