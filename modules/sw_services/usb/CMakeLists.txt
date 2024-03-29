
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    file(GLOB         TUSB_ROOT_SOURCES   thirdparty/tinyusb_src/src/*.c)
    file(GLOB_RECURSE TUSB_CLASS_SOURCES  thirdparty/tinyusb_src/src/class/*/*.c)
    file(GLOB_RECURSE TUSB_COMMON_SOURCES thirdparty/tinyusb_src/src/common/*.c)
    file(GLOB_RECURSE TUSB_DEVICE_SOURCES thirdparty/tinyusb_src/src/device/*.c)

    ## Create library target
    add_library(framework_rtos_sw_services_usb INTERFACE)
    target_sources(framework_rtos_sw_services_usb
        INTERFACE
            portable/dcd_xcore.c
            portable/tusb_nstackwords.S
            FreeRTOS/usb_support.c
            ${TUSB_ROOT_SOURCES}
            ${TUSB_CLASS_SOURCES}
            ${TUSB_COMMON_SOURCES}
            ${TUSB_DEVICE_SOURCES}
    )
    target_include_directories(framework_rtos_sw_services_usb
        INTERFACE
            api
            portable
            thirdparty/tinyusb_src/src
    )
    target_link_libraries(framework_rtos_sw_services_usb
        INTERFACE
            rtos::osal
            rtos::drivers::usb
            rtos::FreeRTOS::FreeRTOS_SMP
    )
    ## These definitions are only needed while we our port is not upstreamed
    target_compile_definitions(framework_rtos_sw_services_usb
        INTERFACE
            CFG_TUSB_MCU=OPT_MCU_NONE
            CFG_TUSB_OS=OPT_OS_CUSTOM
            BOARD_DEVICE_RHPORT_NUM=0
            CFG_TUD_TASK_QUEUE_SZ=8
            TUP_DCD_ENDPOINT_MAX=12  ## RTOS_USB_ENDPOINT_COUNT_MAX
            TUP_RHPORT_HIGHSPEED=0x00
    )

    ## Create an alias
    add_library(rtos::sw_services::usb ALIAS framework_rtos_sw_services_usb)
endif()
