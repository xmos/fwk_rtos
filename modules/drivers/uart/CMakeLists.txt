
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_drivers_uart INTERFACE)
    target_sources(framework_rtos_drivers_uart
        INTERFACE
            src/rtos_uart_tx.c
            src/rtos_uart_tx_rpc.c
            src/rtos_uart_rx.c
    )
    target_include_directories(framework_rtos_drivers_uart
        INTERFACE
            api
    )
    target_link_libraries(framework_rtos_drivers_uart
        INTERFACE
            io::uart
            rtos::osal
    )

    ## Create an alias
    add_library(rtos::drivers::uart ALIAS framework_rtos_drivers_uart)
endif()
