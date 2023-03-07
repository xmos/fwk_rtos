
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_sw_services_dhcp INTERFACE)
    target_sources(framework_rtos_sw_services_dhcp
        INTERFACE
            FreeRTOS/dhcpd.c
    )
    target_include_directories(framework_rtos_sw_services_dhcp
        INTERFACE
            api
            FreeRTOS
    )
    target_link_libraries(framework_rtos_sw_services_dhcp
        INTERFACE
            rtos::FreeRTOS::FreeRTOS-Plus-TCP
    )
    target_compile_definitions(framework_rtos_sw_services_dhcp
        INTERFACE
            USE_DHCPD=1
    )

    ## Create an alias
    add_library(rtos::sw_services::dhcp ALIAS framework_rtos_sw_services_dhcp)
endif()
