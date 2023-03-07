
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_drivers_intertile INTERFACE)
    target_sources(framework_rtos_drivers_intertile
        INTERFACE
            src/rtos_intertile.c
    )
    target_include_directories(framework_rtos_drivers_intertile
        INTERFACE
            api
    )
    target_link_libraries(framework_rtos_drivers_intertile
        INTERFACE
            rtos::osal
    )

    ## Create an alias
    add_library(rtos::drivers::intertile ALIAS framework_rtos_drivers_intertile)
endif()
