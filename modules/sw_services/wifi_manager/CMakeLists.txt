
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_sw_services_wifi_manager INTERFACE)
    target_sources(framework_rtos_sw_services_wifi_manager
        INTERFACE
            FreeRTOS/wifi_connection_mgr.c
    )
    target_include_directories(framework_rtos_sw_services_wifi_manager
        INTERFACE
            api
            FreeRTOS
    )
    target_link_libraries(framework_rtos_sw_services_wifi_manager
        INTERFACE
            rtos::FreeRTOS::FreeRTOS-Plus-TCP
            rtos::sw_services::dhcp
    )

    ## Create an alias
    add_library(rtos::sw_services::wifi_manager ALIAS framework_rtos_sw_services_wifi_manager)
endif()
