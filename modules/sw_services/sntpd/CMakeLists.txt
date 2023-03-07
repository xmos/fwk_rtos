
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_sw_services_sntpd INTERFACE)
    target_sources(framework_rtos_sw_services_sntpd
        INTERFACE
            FreeRTOS/sntpd.c
    )
    target_include_directories(framework_rtos_sw_services_sntpd
        INTERFACE
            FreeRTOS
    )
    target_link_libraries(framework_rtos_sw_services_sntpd
        INTERFACE
            rtos::FreeRTOS::FreeRTOS-Plus-TCP
    )

    ## Create an alias
    add_library(rtos::sw_services::sntpd ALIAS framework_rtos_sw_services_sntpd)
endif()
