
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_drivers_rpc INTERFACE)
    target_sources(framework_rtos_drivers_rpc
        INTERFACE
            src/rtos_rpc.c
    )
    target_include_directories(framework_rtos_drivers_rpc
        INTERFACE
            api
    )
    target_link_libraries(framework_rtos_drivers_rpc
        INTERFACE
            rtos::osal
    )

    ## Create an alias
    add_library(rtos::drivers::rpc ALIAS framework_rtos_drivers_rpc)
endif()
