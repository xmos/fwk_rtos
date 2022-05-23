
## Create library target
add_library(framework_rtos_sw_services_json INTERFACE)
target_include_directories(framework_rtos_sw_services_json
    INTERFACE
        thirdparty/jsmn
)

## Create an alias
add_library(rtos::sw_services::json ALIAS framework_rtos_sw_services_json)
