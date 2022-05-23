
if(DEFINED FRAMEWORK_CORE_ROOT_PATH)
    set(LIB_TFLITE_MICRO_PATH ${FRAMEWORK_CORE_ROOT_PATH}/modules/inferencing/lib_tflite_micro/lib_tflite_micro)
else()
    message(FATAL_ERROR "FRAMEWORK_CORE_ROOT_PATH was not defined")
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## ******************************
    ## Create inferencing RTOS target
    ## ******************************
    add_library(framework_rtos_inferencing_rtos_base STATIC)

    ## Source files
    target_sources(framework_rtos_inferencing_rtos_base
        PRIVATE
            src/xcore_rtos_conv2d_v2.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_custom_options.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_error_reporter.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_interpreter.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_profiler.cc
    )

    target_include_directories(framework_rtos_inferencing_rtos_base
        PRIVATE
            ${LIB_TFLITE_MICRO_PATH}/api
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels
    )
    target_link_libraries(framework_rtos_inferencing_rtos_base
        PUBLIC
            core::tflite_micro
            core::lib_nn
    )

    add_library(framework_rtos_inferencing INTERFACE)

    target_sources(framework_rtos_inferencing
        INTERFACE
            src/debug_log.cc
            src/micro_time.cc
            src/xcore_rtos_load_from_flash.cc
    )

    target_include_directories(framework_rtos_inferencing
        INTERFACE
            ${LIB_TFLITE_MICRO_PATH}/src
            api
    )

    target_link_libraries(framework_rtos_inferencing
        INTERFACE
            framework_rtos_inferencing_rtos_base
            rtos::freertos
    )

    ## Create an alias
    add_library(rtos::sw_services::inferencing ALIAS framework_rtos_inferencing)
endif()
