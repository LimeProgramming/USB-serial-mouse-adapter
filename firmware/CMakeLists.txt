cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(piserialmouse)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Enable autocomplete.
# ln -s build/compile_commands.json .
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# rest of your project
add_executable(pi232mouse
    main.c include/utils.c
)

# Add pico_stdlib library which aggregates commonly used features
target_include_directories(pi232mouse PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(pi232mouse PRIVATE pico_stdlib tinyusb_host tinyusb_board)

include_directories(include/)
link_directories(include/)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(pi232mouse)
