
add_subdirectory(bsp_config)
add_subdirectory(drivers)
add_subdirectory(FreeRTOS)
add_subdirectory(osal)
add_subdirectory(rtos_support)
add_subdirectory(sw_services)

if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Create an aggregate of rtos drivers
    add_library(framework_rtos_drivers_general INTERFACE)
    target_link_libraries(framework_rtos_drivers_general
        INTERFACE
            rtos::drivers::clock_control
            rtos::drivers::gpio
            rtos::drivers::i2c
            rtos::drivers::intertile
            rtos::drivers::l2_cache
            rtos::drivers::qspi_io
            rtos::drivers::rpc
            rtos::drivers::spi
            rtos::drivers::swmem
            rtos::drivers::uart
    )
    add_library(rtos::drivers::general ALIAS framework_rtos_drivers_general)

    ## Create an aggregate of rtos audio drivers
    add_library(framework_rtos_drivers_audio INTERFACE)
    target_link_libraries(framework_rtos_drivers_audio
        INTERFACE
            rtos::drivers::i2s
            rtos::drivers::mic_array
        )
    add_library(rtos::drivers::audio ALIAS framework_rtos_drivers_audio)

    ## Create an aggregate of all rtos drivers
    add_library(framework_rtos_drivers_all INTERFACE)
    target_link_libraries(framework_rtos_drivers_all
        INTERFACE
            rtos::drivers::general
            rtos::drivers::audio
            rtos::drivers::trace
            rtos::drivers::usb
            rtos::drivers::wifi
        )
    add_library(rtos::drivers::all ALIAS framework_rtos_drivers_all)

    ## Create an aggregate of sw_services
    add_library(framework_rtos_sw_services_general INTERFACE)
    target_link_libraries(framework_rtos_sw_services_general
        INTERFACE
            rtos::sw_services::generic_pipeline
            rtos::sw_services::concurrency_support
            rtos::sw_services::fatfs
    )
    add_library(rtos::sw_services::general ALIAS framework_rtos_sw_services_general)

    # Create an aggregate of rtos libraries
    add_library(framework_rtos_freertos INTERFACE)
    target_link_libraries(framework_rtos_freertos
        INTERFACE
            rtos::drivers::general
            rtos::osal
            rtos::rtos_support
            rtos::sw_services::general
            rtos::FreeRTOS::FreeRTOS_SMP
            framework_core_multitile_support
    )
    add_library(rtos::freertos ALIAS framework_rtos_freertos)

    # Create an aggregate of usb device control libraries
    add_library(framework_rtos_usb_device_control INTERFACE)
    target_link_libraries(framework_rtos_usb_device_control
        INTERFACE
            rtos::sw_services::device_control
            rtos::sw_services::usb
    )
    add_library(rtos::usb_device_control ALIAS framework_rtos_usb_device_control)

    # Create an aggregate of wifi libraries
    add_library(framework_rtos_wifi INTERFACE)
    target_link_libraries(framework_rtos_wifi
        INTERFACE
            rtos::drivers::wifi
            rtos::FreeRTOS::FreeRTOS-Plus-TCP
            rtos::sw_services::general
            rtos::sw_services::dhcp
            rtos::sw_services::wifi_manager
    )
    add_library(rtos::wifi ALIAS framework_rtos_wifi)
endif()


if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create an aggregate of iot libraries
    add_library(framework_rtos_iot INTERFACE)
    target_link_libraries(framework_rtos_iot
        INTERFACE
            rtos::freertos
            rtos::wifi
            rtos::sw_services::http
            rtos::sw_services::json
            rtos::sw_services::mqtt
            rtos::sw_services::sntpd
            rtos::sw_services::tls_support
    )

    add_library(rtos::iot ALIAS framework_rtos_iot)

    ## Create an aggregate of rtos usb libraries
    add_library(framework_rtos_usb INTERFACE)
    target_link_libraries(framework_rtos_usb
        INTERFACE
            rtos::freertos
            rtos::sw_services::usb
    )

    add_library(rtos::freertos_usb ALIAS framework_rtos_usb)
endif()
