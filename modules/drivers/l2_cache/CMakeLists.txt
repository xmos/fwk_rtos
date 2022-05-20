
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Create library target
    add_library(framework_rtos_drivers_l2_cache INTERFACE)
    target_sources(framework_rtos_drivers_l2_cache
        INTERFACE
            src/rtos_l2_cache.c
    )
    target_include_directories(framework_rtos_drivers_l2_cache
        INTERFACE
            api
    )
    target_link_libraries(framework_rtos_drivers_l2_cache
        INTERFACE
            core::l2_cache
            rtos::osal
    )

    ## Create an alias
    add_library(rtos::drivers::l2_cache ALIAS framework_rtos_drivers_l2_cache)
endif()
