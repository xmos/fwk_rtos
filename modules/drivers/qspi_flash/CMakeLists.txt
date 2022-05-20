
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_drivers_qspi_io INTERFACE)
    target_sources(framework_rtos_drivers_qspi_io
        INTERFACE
            src/rtos_qspi_flash.c
            src/rtos_qspi_flash_rpc.c
    )
    target_include_directories(framework_rtos_drivers_qspi_io
        INTERFACE
            api
    )
    target_link_libraries(framework_rtos_drivers_qspi_io
        INTERFACE
            io::qspi_io
            rtos::osal
    )

    ## Create an alias
    add_library(rtos::drivers::qspi_io ALIAS framework_rtos_drivers_qspi_io)
endif()
