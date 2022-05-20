
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(framework_rtos_sw_services_mqtt INTERFACE)
    target_sources(framework_rtos_sw_services_mqtt
        INTERFACE
            thirdparty/paho.mqtt.embedded-c/MQTTPacket/src/MQTTConnectClient.c
            thirdparty/paho.mqtt.embedded-c/MQTTPacket/src/MQTTDeserializePublish.c
            thirdparty/paho.mqtt.embedded-c/MQTTPacket/src/MQTTFormat.c
            thirdparty/paho.mqtt.embedded-c/MQTTPacket/src/MQTTPacket.c
            thirdparty/paho.mqtt.embedded-c/MQTTPacket/src/MQTTSerializePublish.c
            thirdparty/paho.mqtt.embedded-c/MQTTPacket/src/MQTTSubscribeClient.c
            thirdparty/paho.mqtt.embedded-c/MQTTPacket/src/MQTTUnsubscribeClient.c
            thirdparty/paho.mqtt.embedded-c/MQTTClient-C/src/MQTTClient.c
            FreeRTOS/MQTTFreeRTOS.c
    )
    target_include_directories(framework_rtos_sw_services_mqtt
        INTERFACE
            thirdparty/paho.mqtt.embedded-c/MQTTPacket/src
            thirdparty/paho.mqtt.embedded-c/MQTTClient-C/src
            FreeRTOS
    )
    target_link_libraries(framework_rtos_sw_services_mqtt
        INTERFACE
            rtos::FreeRTOS::FreeRTOS-Plus-TCP
    )

    ## Create an alias
    add_library(rtos::sw_services::mqtt ALIAS framework_rtos_sw_services_mqtt)
endif()
