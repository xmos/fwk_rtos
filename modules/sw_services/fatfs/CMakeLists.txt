
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_sw_services_fatfs INTERFACE)
    target_sources(framework_rtos_sw_services_fatfs
        INTERFACE
            FreeRTOS/diskio.c
            FreeRTOS/ffsystem.c
            FreeRTOS/fs_support.c
            thirdparty/src/ff.c
            thirdparty/src/ffunicode.c
    )
    target_include_directories(framework_rtos_sw_services_fatfs
        INTERFACE
            FreeRTOS
            thirdparty/api
    )
    target_link_libraries(framework_rtos_sw_services_fatfs
        INTERFACE
            rtos::osal
            rtos::drivers::qspi_io
            rtos::FreeRTOS::FreeRTOS_SMP
    )
    target_compile_definitions(framework_rtos_sw_services_fatfs
        INTERFACE
            USE_FATFS=1
    )

    ## Create an alias
    add_library(rtos::sw_services::fatfs ALIAS framework_rtos_sw_services_fatfs)
endif()
