SET(LIB_TFLITE_MICRO_PATH ${PROJECT_SOURCE_DIR}/modules/inferencing/lib_tflite_micro/lib_tflite_micro)

if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## ******************************
    ## Create inferencing RTOS target
    ## ******************************
    add_library(xcore_sdk_inferencing_rtos_base STATIC)

    ## Source files
    target_sources(xcore_sdk_inferencing_rtos_base
        PRIVATE
            src/xcore_rtos_conv2d_v2.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_custom_options.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_error_reporter.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_interpreter.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_profiler.cc
    )

    target_include_directories(xcore_sdk_inferencing_rtos_base
        PRIVATE
            ${LIB_TFLITE_MICRO_PATH}/api
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels
    )
    target_link_libraries(xcore_sdk_inferencing_rtos_base
        PUBLIC
            sdk::inferencing::tflite_micro
            sdk::inferencing::lib_nn
    )

    add_library(xcore_sdk_inferencing_rtos INTERFACE)

    target_sources(xcore_sdk_inferencing_rtos
        INTERFACE
            src/debug_log.cc
            src/micro_time.cc
            src/xcore_rtos_load_from_flash.cc
    )

    target_include_directories(xcore_sdk_inferencing_rtos_base
        INTERFACE
            ${LIB_TFLITE_MICRO_PATH}/src
            api
    )

    target_link_libraries(xcore_sdk_inferencing_rtos
        INTERFACE
            xcore_sdk_inferencing_rtos_base
            sdk::rtos_freertos
    )

    ## Create an alias
    add_library(sdk::inferencing::rtos ALIAS xcore_sdk_inferencing_rtos)
endif()

