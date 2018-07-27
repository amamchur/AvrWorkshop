cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)

project(AvrWorkshop)

include_directories(/usr/local/include)

add_mcu_executable(Neotronica atmega328p
        apps/Neotronica/MatrixHelpers.hpp
        apps/Neotronica/main.cpp
        )
target_compile_definitions(Neotronica.elf PRIVATE F_CPU=16000000UL ARDUINO_AVR_PRO=1)

add_mcu_executable(Keyboard atmega32u4
        libs/LUFA/Drivers/USB/Class/Device/HIDClassDevice.c
        libs/LUFA/Drivers/USB/Core/Events.c
        libs/LUFA/Drivers/USB/Core/DeviceStandardReq.c
        libs/LUFA/Drivers/USB/Core/USBTask.c
        libs/LUFA/Drivers/USB/Core/AVR8/USBController_AVR8.c
        libs/LUFA/Drivers/USB/Core/AVR8/USBInterrupt_AVR8.c
        libs/LUFA/Drivers/USB/Core/AVR8/Device_AVR8.c
        libs/LUFA/Drivers/USB/Core/AVR8/Endpoint_AVR8.c
        libs/LUFA/Drivers/USB/Core/AVR8/EndpointStream_AVR8.c
        apps/Keyboard/Descriptors.c
        apps/Keyboard/main.cpp
        )
target_compile_definitions(Keyboard.elf PRIVATE
        F_CPU=16000000UL
        F_USB=16000000UL
        USE_LUFA_CONFIG_HEADER=1
        )
target_include_directories(Keyboard.elf PRIVATE libs apps/Keyboard/Config)
