cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)

project(AvrWorkshop)

include_directories(/usr/local/include)

add_mcu_executable(Neotronica atmega328p
        apps/Neotronica/AppConfig.hpp
        apps/Neotronica/MatrixHelpers.hpp
        apps/Neotronica/main.cpp
        )
target_compile_definitions(Neotronica.elf PRIVATE F_CPU=16000000UL ARDUINO_AVR_PRO=1)
