
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_drivers_i2s INTERFACE)
    target_sources(framework_rtos_drivers_i2s
        INTERFACE
            src/rtos_i2s.c
            src/rtos_i2s_rpc.c
    )
    target_include_directories(framework_rtos_drivers_i2s
        INTERFACE
            api
    )
    target_link_libraries(framework_rtos_drivers_i2s
        INTERFACE
            lib_i2s
            rtos::osal
    )

    ## Create an alias
    add_library(rtos::drivers::i2s ALIAS framework_rtos_drivers_i2s)
endif()
