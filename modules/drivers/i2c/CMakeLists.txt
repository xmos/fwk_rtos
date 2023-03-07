
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_drivers_i2c INTERFACE)
    target_sources(framework_rtos_drivers_i2c
        INTERFACE
            src/rtos_i2c_master.c
            src/rtos_i2c_master_rpc.c
            src/rtos_i2c_slave.c
    )
    target_include_directories(framework_rtos_drivers_i2c
        INTERFACE
            api
    )
    target_link_libraries(framework_rtos_drivers_i2c
        INTERFACE
            lib_i2c
            rtos::osal
    )

    ## Create an alias
    add_library(rtos::drivers::i2c ALIAS framework_rtos_drivers_i2c)
endif()
