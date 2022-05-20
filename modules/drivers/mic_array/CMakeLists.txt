if(DEFINED FRAMEWORK_IO_ROOT_PATH)
    set(LIB_MIC_ARRAY_PATH ${FRAMEWORK_IO_ROOT_PATH}/modules/mic_array)
else()
    message(FATAL_ERROR "FRAMEWORK_IO_ROOT_PATH was not defined")
endif()

# Note, the inclusion of the vanilla API stuff below is a temporary hack until
# a more RTOS-friendly API is available for the mic array
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_drivers_mic_array INTERFACE)
    target_sources(framework_rtos_drivers_mic_array
        INTERFACE
            src/rtos_mic_array.c
            src/rtos_mic_array_rpc.c
            ${LIB_MIC_ARRAY_PATH}/etc/vanilla/mic_array_vanilla.cpp
    )
    target_include_directories(framework_rtos_drivers_mic_array
        INTERFACE
            api
            ${LIB_MIC_ARRAY_PATH}/etc/vanilla/
    )
    target_link_libraries(framework_rtos_drivers_mic_array
        INTERFACE
            io::mic_array
            rtos::osal
    )
    target_compile_definitions(framework_rtos_drivers_mic_array
        INTERFACE
            MIC_ARRAY_BASIC_API_ENABLE=1
    )

    ## Create an alias
    add_library(rtos::drivers::mic_array ALIAS framework_rtos_drivers_mic_array)
endif()
