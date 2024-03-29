cmake_minimum_required(VERSION 3.20)

# Compile for x86_64 on Mac as we can't support the M1 ARM architecture yet
set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE INTERNAL "")

project(datapartition_mkimage LANGUAGES C)
set(TARGET_NAME datapartition_mkimage)

# Determine OS, set up output dirs
if(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
    set(DATAPARTITION_INSTALL_DIR "/opt/xmos/bin")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
    set(DATAPARTITION_INSTALL_DIR "/opt/xmos/bin")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set(DATAPARTITION_INSTALL_DIR "$ENV{USERPROFILE}\\.xmos\\bin")
endif()

set(APP_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/${TARGET_NAME}.c"
)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES})
install(TARGETS ${TARGET_NAME} DESTINATION ${DATAPARTITION_INSTALL_DIR})

if ((CMAKE_C_COMPILER_ID STREQUAL "Clang") OR (CMAKE_C_COMPILER_ID STREQUAL "AppleClang"))
    message(STATUS "Configuring for Clang")
    target_compile_options(${TARGET_NAME} PRIVATE -O2 -Wall)
    target_link_options(${TARGET_NAME} PRIVATE "")
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Configuring for GCC")
    target_compile_options(${TARGET_NAME} PRIVATE -O2 -Wall)
    target_link_options(${TARGET_NAME} PRIVATE "")
elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Configuring for MSVC")
    target_compile_options(${TARGET_NAME} PRIVATE /W3)
    target_link_options(${TARGET_NAME} PRIVATE "")
else ()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_C_COMPILER_ID}")
endif()
