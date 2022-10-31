
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_drivers_trace INTERFACE)
    file(GLOB TRACEALYZER_SOURCES
        FreeRTOS/tracealyzer/*.c
        FreeRTOS/tracealyzer/streamports/XMOS_xScope/*.c
        FreeRTOS/tracealyzer/kernelports/FreeRTOS/*.c
    )
    target_sources(framework_rtos_drivers_trace
        INTERFACE
            FreeRTOS/ASCII/ascii_trace.c
            ${TRACEALYZER_SOURCES}
    )
    target_include_directories(framework_rtos_drivers_trace
        INTERFACE
        FreeRTOS/api
        FreeRTOS/tracealyzer/include
        FreeRTOS/tracealyzer/streamports/XMOS_xScope/include
        FreeRTOS/tracealyzer/kernelports/FreeRTOS/include
    )
    target_link_libraries(framework_rtos_drivers_trace
        INTERFACE
            rtos::FreeRTOS::FreeRTOS_SMP
    )
    target_compile_definitions(framework_rtos_drivers_trace
        INTERFACE
            portBASE_TYPE=int
    )

    ## Create an alias
    add_library(rtos::drivers::trace ALIAS framework_rtos_drivers_trace)
endif()
