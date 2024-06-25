if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Create FreeRTOS kernel library target
    add_library(framework_rtos_freertos_smp INTERFACE)
    target_sources(framework_rtos_freertos_smp
        INTERFACE
            FreeRTOS-SMP-Kernel/portable/ThirdParty/xClang/XCOREAI/port.xc
            FreeRTOS-SMP-Kernel/croutine.c
            FreeRTOS-SMP-Kernel/event_groups.c
            FreeRTOS-SMP-Kernel/list.c
            FreeRTOS-SMP-Kernel/queue.c
            FreeRTOS-SMP-Kernel/stream_buffer.c
            FreeRTOS-SMP-Kernel/tasks.c
            FreeRTOS-SMP-Kernel/timers.c
            FreeRTOS-SMP-Kernel/portable/ThirdParty/xClang/XCOREAI/port.c
            FreeRTOS-SMP-Kernel/portable/MemMang/heap_4.c
            FreeRTOS-SMP-Kernel/portable/ThirdParty/xClang/XCOREAI/portasm.S
    )
    target_include_directories(framework_rtos_freertos_smp
        INTERFACE
            FreeRTOS-SMP-Kernel/include
            FreeRTOS-SMP-Kernel/portable/ThirdParty/xClang/XCOREAI
    )
    # Use O2 for tasks.c to give it a slight speedup
    set_source_files_properties(FreeRTOS-SMP-Kernel/tasks.c PROPERTIES COMPILE_FLAGS "-O2")

    ## Create an alias
    add_library(rtos::FreeRTOS::FreeRTOS_SMP ALIAS framework_rtos_freertos_smp)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A)
    ## Create FreeRTOS kernel library target
    add_library(framework_rtos_freertos_smp INTERFACE)
    target_sources(framework_rtos_freertos_smp
        INTERFACE
            FreeRTOS-SMP-Kernel/portable/ThirdParty/xClang/XCORE200/port.xc
            FreeRTOS-SMP-Kernel/croutine.c
            FreeRTOS-SMP-Kernel/event_groups.c
            FreeRTOS-SMP-Kernel/list.c
            FreeRTOS-SMP-Kernel/queue.c
            FreeRTOS-SMP-Kernel/stream_buffer.c
            FreeRTOS-SMP-Kernel/tasks.c
            FreeRTOS-SMP-Kernel/timers.c
            FreeRTOS-SMP-Kernel/portable/ThirdParty/xClang/XCORE200/port.c
            FreeRTOS-SMP-Kernel/portable/MemMang/heap_4.c
            FreeRTOS-SMP-Kernel/portable/ThirdParty/xClang/XCORE200/portasm.S
    )
    target_include_directories(framework_rtos_freertos_smp
        INTERFACE
            FreeRTOS-SMP-Kernel/include
            FreeRTOS-SMP-Kernel/portable/ThirdParty/xClang/XCORE200
    )
    # Use O2 for tasks.c to give it a slight speedup
    set_source_files_properties(FreeRTOS-SMP-Kernel/tasks.c PROPERTIES COMPILE_FLAGS "-O2")

    ## Create an alias
    add_library(rtos::FreeRTOS::FreeRTOS_SMP ALIAS framework_rtos_freertos_smp)
else()
    message(STATUS "FreeRTOS port does not exist for ${CMAKE_SYSTEM_NAME}!  No RTOS targets made.")
endif()



if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create FreeRTOS-Plus-TCP target
    ## In FreeRTOS-Plus-TCP v3.0.0 and v3.1.0, the folder structure dramatically
    ##    changed and some source files were split. Create targets for the
    ##    split files to aid compatibility efforts. In the future, consider
    ##    only using ones that we need for e.g. tests.

    add_library(framework_rtos_freertos_plus_tcp_arp INTERFACE)
    target_sources(framework_rtos_freertos_plus_tcp_arp
        INTERFACE
            FreeRTOS-Plus-TCP/source/FreeRTOS_ARP.c
            FreeRTOS-Plus-TCP/source/portable/BufferManagement/BufferAllocation_2.c
            portable/FreeRTOS-Plus-TCP/NetworkInterface/FreeRTOS_TCP_port.c
    )
    target_include_directories(framework_rtos_freertos_plus_tcp_arp
        INTERFACE
            FreeRTOS-Plus-TCP/source/include
            portable/FreeRTOS-Plus-TCP/Compiler
            portable/FreeRTOS-Plus-TCP/NetworkInterface
    )
    target_link_libraries(framework_rtos_freertos_plus_tcp_arp
        INTERFACE
            rtos::FreeRTOS::FreeRTOS_SMP
            lib_random
    )

    ## Create an alias
    add_library(rtos::FreeRTOS::FreeRTOS-Plus-TCP::ARP ALIAS framework_rtos_freertos_plus_tcp_arp)

    add_library(framework_rtos_freertos_plus_tcp_dhcp INTERFACE)
    target_sources(framework_rtos_freertos_plus_tcp_dhcp
        INTERFACE
            FreeRTOS-Plus-TCP/source/FreeRTOS_DHCP.c
            FreeRTOS-Plus-TCP/source/portable/BufferManagement/BufferAllocation_2.c
            portable/FreeRTOS-Plus-TCP/NetworkInterface/FreeRTOS_TCP_port.c
    )
    target_include_directories(framework_rtos_freertos_plus_tcp_dhcp
        INTERFACE
            FreeRTOS-Plus-TCP/source/include
            portable/FreeRTOS-Plus-TCP/Compiler
            portable/FreeRTOS-Plus-TCP/NetworkInterface
    )
    target_link_libraries(framework_rtos_freertos_plus_tcp_dhcp
        INTERFACE
            rtos::FreeRTOS::FreeRTOS_SMP
            lib_random
    )

    ## Create an alias
    add_library(rtos::FreeRTOS::FreeRTOS-Plus-TCP::DHCP ALIAS framework_rtos_freertos_plus_tcp_dhcp)

    add_library(framework_rtos_freertos_plus_tcp_dns INTERFACE)
    target_sources(framework_rtos_freertos_plus_tcp_dns
        INTERFACE
            FreeRTOS-Plus-TCP/source/FreeRTOS_DNS.c
            FreeRTOS-Plus-TCP/source/FreeRTOS_DNS_Cache.c
            FreeRTOS-Plus-TCP/source/FreeRTOS_DNS_Callback.c
            FreeRTOS-Plus-TCP/source/FreeRTOS_DNS_Networking.c
            FreeRTOS-Plus-TCP/source/FreeRTOS_DNS_Parser.c
            FreeRTOS-Plus-TCP/source/portable/BufferManagement/BufferAllocation_2.c
            portable/FreeRTOS-Plus-TCP/NetworkInterface/FreeRTOS_TCP_port.c
    )
    target_include_directories(framework_rtos_freertos_plus_tcp_dns
        INTERFACE
            FreeRTOS-Plus-TCP/source/include
            portable/FreeRTOS-Plus-TCP/Compiler
            portable/FreeRTOS-Plus-TCP/NetworkInterface
    )
    target_link_libraries(framework_rtos_freertos_plus_tcp_dns
        INTERFACE
            rtos::FreeRTOS::FreeRTOS_SMP
            lib_random
    )

    ## Create an alias
    add_library(rtos::FreeRTOS::FreeRTOS-Plus-TCP::DNS ALIAS framework_rtos_freertos_plus_tcp_dns)

    add_library(framework_rtos_freertos_plus_tcp_ip INTERFACE)
    target_sources(framework_rtos_freertos_plus_tcp_ip
        INTERFACE
            FreeRTOS-Plus-TCP/source/FreeRTOS_ICMP.c
            FreeRTOS-Plus-TCP/source/FreeRTOS_IP.c
            FreeRTOS-Plus-TCP/source/FreeRTOS_IP_Timers.c
            FreeRTOS-Plus-TCP/source/FreeRTOS_IP_Utils.c
            FreeRTOS-Plus-TCP/source/portable/BufferManagement/BufferAllocation_2.c
            portable/FreeRTOS-Plus-TCP/NetworkInterface/FreeRTOS_TCP_port.c
    )
    target_include_directories(framework_rtos_freertos_plus_tcp_ip
        INTERFACE
            FreeRTOS-Plus-TCP/source/include
            portable/FreeRTOS-Plus-TCP/Compiler
            portable/FreeRTOS-Plus-TCP/NetworkInterface
    )
    target_link_libraries(framework_rtos_freertos_plus_tcp_ip
        INTERFACE
            rtos::FreeRTOS::FreeRTOS_SMP
            lib_random
    )

    ## Create an alias
    add_library(rtos::FreeRTOS::FreeRTOS-Plus-TCP::IP ALIAS framework_rtos_freertos_plus_tcp_ip)

    add_library(framework_rtos_freertos_plus_tcp_sockets INTERFACE)
    target_sources(framework_rtos_freertos_plus_tcp_sockets
        INTERFACE
            FreeRTOS-Plus-TCP/source/FreeRTOS_Sockets.c
            FreeRTOS-Plus-TCP/source/portable/BufferManagement/BufferAllocation_2.c
            portable/FreeRTOS-Plus-TCP/NetworkInterface/FreeRTOS_TCP_port.c
    )
    target_include_directories(framework_rtos_freertos_plus_tcp_sockets
        INTERFACE
            FreeRTOS-Plus-TCP/source/include
            portable/FreeRTOS-Plus-TCP/Compiler
            portable/FreeRTOS-Plus-TCP/NetworkInterface
    )
    target_link_libraries(framework_rtos_freertos_plus_tcp_sockets
        INTERFACE
            rtos::FreeRTOS::FreeRTOS_SMP
            lib_random
    )

    ## Create an alias
    add_library(rtos::FreeRTOS::FreeRTOS-Plus-TCP::Sockets ALIAS framework_rtos_freertos_plus_tcp_sockets)

    add_library(framework_rtos_freertos_plus_tcp_stream_buffer INTERFACE)
    target_sources(framework_rtos_freertos_plus_tcp_stream_buffer
        INTERFACE
            FreeRTOS-Plus-TCP/source/FreeRTOS_Stream_Buffer.c
            FreeRTOS-Plus-TCP/source/portable/BufferManagement/BufferAllocation_2.c
            portable/FreeRTOS-Plus-TCP/NetworkInterface/FreeRTOS_TCP_port.c
    )
    target_include_directories(framework_rtos_freertos_plus_tcp_stream_buffer
        INTERFACE
            FreeRTOS-Plus-TCP/source/include
            portable/FreeRTOS-Plus-TCP/Compiler
            portable/FreeRTOS-Plus-TCP/NetworkInterface
    )
    target_link_libraries(framework_rtos_freertos_plus_tcp_stream_buffer
        INTERFACE
            rtos::FreeRTOS::FreeRTOS_SMP
            lib_random
    )

    ## Create an alias
    add_library(rtos::FreeRTOS::FreeRTOS-Plus-TCP::Stream-Buffer ALIAS framework_rtos_freertos_plus_tcp_stream_buffer)

    add_library(framework_rtos_freertos_plus_tcp_tcp_ip INTERFACE)
    target_sources(framework_rtos_freertos_plus_tcp_tcp_ip
        INTERFACE
            FreeRTOS-Plus-TCP/source/FreeRTOS_TCP_IP.c
            FreeRTOS-Plus-TCP/source/FreeRTOS_TCP_Reception.c
            FreeRTOS-Plus-TCP/source/FreeRTOS_TCP_State_Handling.c
            FreeRTOS-Plus-TCP/source/FreeRTOS_TCP_Transmission.c
            FreeRTOS-Plus-TCP/source/FreeRTOS_TCP_Utils.c
            FreeRTOS-Plus-TCP/source/portable/BufferManagement/BufferAllocation_2.c
            portable/FreeRTOS-Plus-TCP/NetworkInterface/FreeRTOS_TCP_port.c
    )
    target_include_directories(framework_rtos_freertos_plus_tcp_tcp_ip
        INTERFACE
            FreeRTOS-Plus-TCP/source/include
            portable/FreeRTOS-Plus-TCP/Compiler
            portable/FreeRTOS-Plus-TCP/NetworkInterface
    )
    target_link_libraries(framework_rtos_freertos_plus_tcp_tcp_ip
        INTERFACE
            rtos::FreeRTOS::FreeRTOS_SMP
            lib_random
    )

    ## Create an alias
    add_library(rtos::FreeRTOS::FreeRTOS-Plus-TCP::TCP-IP ALIAS framework_rtos_freertos_plus_tcp_tcp_ip)

    add_library(framework_rtos_freertos_plus_tcp_tcp_win INTERFACE)
    target_sources(framework_rtos_freertos_plus_tcp_tcp_win
        INTERFACE
            FreeRTOS-Plus-TCP/source/FreeRTOS_TCP_WIN.c
            FreeRTOS-Plus-TCP/source/FreeRTOS_Tiny_TCP.c
            FreeRTOS-Plus-TCP/source/portable/BufferManagement/BufferAllocation_2.c
            portable/FreeRTOS-Plus-TCP/NetworkInterface/FreeRTOS_TCP_port.c
    )
    target_include_directories(framework_rtos_freertos_plus_tcp_tcp_win
        INTERFACE
            FreeRTOS-Plus-TCP/source/include
            portable/FreeRTOS-Plus-TCP/Compiler
            portable/FreeRTOS-Plus-TCP/NetworkInterface
    )
    target_link_libraries(framework_rtos_freertos_plus_tcp_tcp_win
        INTERFACE
            rtos::FreeRTOS::FreeRTOS_SMP
            lib_random
    )

    ## Create an alias
    add_library(rtos::FreeRTOS::FreeRTOS-Plus-TCP::TCP-WIN ALIAS framework_rtos_freertos_plus_tcp_tcp_win)

    add_library(framework_rtos_freertos_plus_tcp_udp_ip INTERFACE)
    target_sources(framework_rtos_freertos_plus_tcp_udp_ip
        INTERFACE
            FreeRTOS-Plus-TCP/source/FreeRTOS_UDP_IP.c
            FreeRTOS-Plus-TCP/source/portable/BufferManagement/BufferAllocation_2.c
            portable/FreeRTOS-Plus-TCP/NetworkInterface/FreeRTOS_TCP_port.c
    )
    target_include_directories(framework_rtos_freertos_plus_tcp_udp_ip
        INTERFACE
            FreeRTOS-Plus-TCP/source/include
            portable/FreeRTOS-Plus-TCP/Compiler
            portable/FreeRTOS-Plus-TCP/NetworkInterface
    )
    target_link_libraries(framework_rtos_freertos_plus_tcp_udp_ip
        INTERFACE
            rtos::FreeRTOS::FreeRTOS_SMP
            lib_random
    )

    ## Create an alias
    add_library(rtos::FreeRTOS::FreeRTOS-Plus-TCP::UDP-IP ALIAS framework_rtos_freertos_plus_tcp_udp_ip)

    add_library(framework_rtos_freertos_plus_tcp INTERFACE)
    target_link_libraries(framework_rtos_freertos_plus_tcp
        INTERFACE
            rtos::FreeRTOS::FreeRTOS-Plus-TCP::ARP
            rtos::FreeRTOS::FreeRTOS-Plus-TCP::DHCP
            rtos::FreeRTOS::FreeRTOS-Plus-TCP::DNS
            rtos::FreeRTOS::FreeRTOS-Plus-TCP::IP
            rtos::FreeRTOS::FreeRTOS-Plus-TCP::Sockets
            rtos::FreeRTOS::FreeRTOS-Plus-TCP::Stream-Buffer
            rtos::FreeRTOS::FreeRTOS-Plus-TCP::TCP-IP
            rtos::FreeRTOS::FreeRTOS-Plus-TCP::TCP-WIN
            rtos::FreeRTOS::FreeRTOS-Plus-TCP::UDP-IP
    )

    ## Create an alias
    add_library(rtos::FreeRTOS::FreeRTOS-Plus-TCP ALIAS framework_rtos_freertos_plus_tcp)
endif()
