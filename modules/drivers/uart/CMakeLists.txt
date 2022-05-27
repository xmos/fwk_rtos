
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_drivers_uart INTERFACE)
    target_sources(xcore_sdk_rtos_drivers_uart
        INTERFACE
            src/rtos_uart_tx.c
            # src/rtos_i2c_master_rpc.c
            src/rtos_uart_rx.c
    )
    target_include_directories(xcore_sdk_rtos_drivers_uart
        INTERFACE
            api
    )
    target_link_libraries(xcore_sdk_rtos_drivers_uart
        INTERFACE
            sdk::hil::lib_uart
            sdk::rtos::osal
    )

    ## Create an alias
    add_library(sdk::rtos::drivers::uart ALIAS xcore_sdk_rtos_drivers_uart)
endif()
