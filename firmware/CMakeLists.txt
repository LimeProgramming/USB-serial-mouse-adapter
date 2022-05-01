# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.13)

set(TARGET "usb-2-232")
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "C:/Pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

#if(NOT CMAKE_BUILD_TYPE)
#  set(CMAKE_BUILD_TYPE Release)
#endif()

#set( CMAKE_CXX_FLAGS "-Wall -Wextra")
#set( CMAKE_C_FLAGS "-Wall -Wextra")
set( CMAKE_C_FLAGS_RELEASE "-O2" )
set( CMAKE_CXX_FLAGS_RELEASE "-O2" )

project(usb-2-232 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
#add_executable(usb-2-232 ${CMAKE_CURRENT_LIST_DIR})
add_executable(usb-2-232 
  usb-2-232.c
  include/hid_app.c
  include/serial.c
  include/utils.c
  include/core_1.c
)

# Make sure TinyUSB can find tusb_config.h
target_include_directories(usb-2-232 PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_host
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(usb-2-232 pico_stdlib hardware_flash hardware_sync tinyusb_host tinyusb_board pico_multicore)

include_directories(include/)
link_directories(include/)

pico_set_program_name(${TARGET} "usb-2-232")
pico_set_program_description(${TARGET} "Pico usb to serial mouse adaptor.")
pico_set_program_version(${TARGET} "0.1")

pico_enable_stdio_usb(usb-2-232 0)
pico_enable_stdio_uart(usb-2-232 1)

pico_add_extra_outputs(usb-2-232)

