
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
## Create library target
add_library(framework_rtos_drivers_dfu_image INTERFACE)
target_sources(framework_rtos_drivers_dfu_image
    INTERFACE
        src/rtos_dfu_image.c
)
target_include_directories(framework_rtos_drivers_dfu_image
    INTERFACE
        api
)
target_link_libraries(framework_rtos_drivers_dfu_image
    INTERFACE
        rtos::osal
)
target_link_options(framework_rtos_drivers_dfu_image
    INTERFACE
        -lquadflash
)

## Create an alias
add_library(rtos::drivers::dfu_image ALIAS framework_rtos_drivers_dfu_image)
endif()
