cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pi232mouse)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(pi232mouse)

# Example source
target_sources(pi232mouse PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.c 
        ${CMAKE_CURRENT_LIST_DIR}/include/utils.c
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(pi232mouse PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

include_directories(include/)
link_directories(include/)

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_host
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(pi232mouse PUBLIC pico_stdlib tinyusb_host tinyusb_board)

pico_add_extra_outputs(pi232mouse)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

# add url via pico_set_program_url
#example_auto_set_url(pi232mouse)