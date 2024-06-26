if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Create FreeRTOS kernel library target
    add_library(framework_rtos_freertos_smp INTERFACE)
    target_sources(framework_rtos_freertos_smp
        INTERFACE
            FreeRTOS-Kernel/portable/ThirdParty/xClang/XCOREAI/port.xc
            FreeRTOS-Kernel/croutine.c
            FreeRTOS-Kernel/event_groups.c
            FreeRTOS-Kernel/list.c
            FreeRTOS-Kernel/queue.c
            FreeRTOS-Kernel/stream_buffer.c
            FreeRTOS-Kernel/tasks.c
            FreeRTOS-Kernel/timers.c
            FreeRTOS-Kernel/portable/ThirdParty/xClang/XCOREAI/port.c
            FreeRTOS-Kernel/portable/MemMang/heap_4.c
            FreeRTOS-Kernel/portable/ThirdParty/xClang/XCOREAI/portasm.S
    )
    target_include_directories(framework_rtos_freertos_smp
        INTERFACE
            FreeRTOS-Kernel/include
            FreeRTOS-Kernel/portable/ThirdParty/xClang/XCOREAI
    )
    # Use O2 for tasks.c to give it a slight speedup
    set_source_files_properties(FreeRTOS-Kernel/tasks.c PROPERTIES COMPILE_FLAGS "-O2")

    ## Create an alias
    add_library(rtos::FreeRTOS::FreeRTOS ALIAS framework_rtos_freertos_smp)
else()
    message(STATUS "FreeRTOS port does not exist for ${CMAKE_SYSTEM_NAME}!  No RTOS targets made.")
endif()



if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Create FreeRTOS-Plus-TCP target
    add_library(framework_rtos_freertos_plus_tcp INTERFACE)
    target_sources(framework_rtos_freertos_plus_tcp
        INTERFACE
            FreeRTOS-Plus-TCP/FreeRTOS_ARP.c
            FreeRTOS-Plus-TCP/FreeRTOS_DHCP.c
            FreeRTOS-Plus-TCP/FreeRTOS_DNS.c
            FreeRTOS-Plus-TCP/FreeRTOS_IP.c
            FreeRTOS-Plus-TCP/FreeRTOS_Sockets.c
            FreeRTOS-Plus-TCP/FreeRTOS_Stream_Buffer.c
            FreeRTOS-Plus-TCP/FreeRTOS_TCP_IP.c
            FreeRTOS-Plus-TCP/FreeRTOS_TCP_WIN.c
            FreeRTOS-Plus-TCP/FreeRTOS_UDP_IP.c
            FreeRTOS-Plus-TCP/portable/BufferManagement/BufferAllocation_2.c
            portable/FreeRTOS-Plus-TCP/NetworkInterface/FreeRTOS_TCP_port.c
    )
    target_include_directories(framework_rtos_freertos_plus_tcp
        INTERFACE
            FreeRTOS-Plus-TCP/include
            portable/FreeRTOS-Plus-TCP/Compiler
            portable/FreeRTOS-Plus-TCP/NetworkInterface
    )
    target_link_libraries(framework_rtos_freertos_plus_tcp
        INTERFACE
            rtos::FreeRTOS::FreeRTOS
            lib_random
    )

    ## Create an alias
    add_library(rtos::FreeRTOS::FreeRTOS-Plus-TCP ALIAS framework_rtos_freertos_plus_tcp)
endif()
