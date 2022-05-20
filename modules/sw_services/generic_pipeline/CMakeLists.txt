
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_sw_services_generic_pipeline INTERFACE)
    target_sources(framework_rtos_sw_services_generic_pipeline
        INTERFACE
            src/generic_pipeline.c
    )
    target_include_directories(framework_rtos_sw_services_generic_pipeline
        INTERFACE
            api
    )
    target_link_libraries(framework_rtos_sw_services_generic_pipeline
        INTERFACE
            rtos::osal
    )

    ## Create an alias
    add_library(rtos::sw_services::generic_pipeline ALIAS framework_rtos_sw_services_generic_pipeline)
endif()
