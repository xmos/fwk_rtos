
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    
    file(GLOB_RECURSE MBEDTLS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/thirdparty/mbedtls/library/*.c )
    
    ## Create library target
    add_library(framework_rtos_sw_services_tls INTERFACE)
    target_sources(framework_rtos_sw_services_tls
        INTERFACE
            FreeRTOS/mbedtls_support.c
            thirdparty/port/mbedtls/FreeRTOS/mbedtls_xcore_platform.c
            ${MBEDTLS_SOURCES}
    )
    target_include_directories(framework_rtos_sw_services_tls
        INTERFACE
            api
            FreeRTOS/api
            thirdparty/mbedtls/include
            thirdparty/port/mbedtls/FreeRTOS
    )
    target_link_libraries(framework_rtos_sw_services_tls
        INTERFACE
            rtos::FreeRTOS::FreeRTOS-Plus-TCP
            rtos::sw_services::fatfs
            lib_random
    )

    ## Create an alias
    add_library(rtos::sw_services::tls_support ALIAS framework_rtos_sw_services_tls)
endif()
