
## Create library target
add_library(framework_rtos_sw_services_http INTERFACE)
target_sources(framework_rtos_sw_services_http
    INTERFACE
        thirdparty/coreHTTP/source/dependency/3rdparty/http_parser/http_parser.c
)
target_include_directories(framework_rtos_sw_services_http
    INTERFACE
        thirdparty/coreHTTP/source/dependency/3rdparty/http_parser
)

## Create an alias
add_library(rtos::sw_services::http ALIAS framework_rtos_sw_services_http)
