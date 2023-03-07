
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Create library target
    add_library(framework_rtos_drivers_swmem INTERFACE)
    target_sources(framework_rtos_drivers_swmem
        INTERFACE
            src/rtos_swmem.c
    )
    target_include_directories(framework_rtos_drivers_swmem
        INTERFACE
            api
    )
    target_link_libraries(framework_rtos_drivers_swmem
        INTERFACE
            rtos::osal
    )

    ## Create an alias
    add_library(rtos::drivers::swmem ALIAS framework_rtos_drivers_swmem)
endif()
