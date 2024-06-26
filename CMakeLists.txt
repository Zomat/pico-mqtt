# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.19)
set(PICO_SDK_PATH /Users/mateuszzolisz/pico-sdk)
# Include build functions from Pico SDK
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C   standards
project(blink C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Tell CMake where to find the executable source file
file(GLOB APP_SOURCES "app/*.c")
file(GLOB APP_SOURCES "drivers/*.c")
add_executable(${PROJECT_NAME} 
    main.c
    ${APP_SOURCES}
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib hardware_gpio hardware_timer
    pico_cyw43_arch_none
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
    hardware_i2c
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
