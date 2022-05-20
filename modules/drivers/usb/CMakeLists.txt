
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_drivers_usb INTERFACE)
    target_sources(framework_rtos_drivers_usb
        INTERFACE
            src/rtos_usb.c
            src/xud_xfer_data.S
    )
    set_source_files_properties(src/xud_xfer_data.S PROPERTIES LANGUAGE ASM)
    target_include_directories(framework_rtos_drivers_usb
        INTERFACE
            api
            src
    )
    target_link_libraries(framework_rtos_drivers_usb
        INTERFACE
            io::xud
            rtos::osal
    )

    ## Create an alias
    add_library(rtos::drivers::usb ALIAS framework_rtos_drivers_usb)
endif()
