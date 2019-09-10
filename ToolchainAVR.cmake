set(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_C_COMPILER_WORKS 1)

set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_CXX_COMPILER_WORKS 1)

find_program(AVR_OBJCOPY avr-objcopy)
find_program(AVR_OBJDUMP avr-objdump)
find_program(AVR_SIZE avr-size)

function(add_avr_executable NAME MCU)
    if (NOT ARGN)
        message(FATAL_ERROR "No source files given for ${NAME}.")
    endif (NOT ARGN)

    set(exec_id ${NAME})
    set(bin_file ${exec_id}.bin)
    set(map_file ${exec_id}.map)
    set(lst_file ${exec_id}.lst)
    set(eep_file ${exec_id}.eep)

    add_executable(${NAME}.elf ${ARGN})

    set_target_properties(
            ${NAME}.elf
            PROPERTIES
            LINK_FLAGS "-mmcu=${MCU} -Wl,--gc-sections -mrelax -Wl,-Map,${map_file}"
    )

    target_compile_options(
            ${NAME}.elf
            PRIVATE
            -mmcu=${MCU}
            -Wall
            -Wno-unknown-pragmas
            -fno-exceptions
            -ffunction-sections
    )

    add_custom_command(
            TARGET ${NAME}.elf
            POST_BUILD
            COMMAND ${AVR_OBJCOPY} -j .text -j .data -O binary ${NAME}.elf ${bin_file}
            COMMAND ${AVR_OBJDUMP} -h -S ${NAME}.elf > ${lst_file}
            COMMAND ${AVR_OBJCOPY}
            --only-section .eeprom
            --set-section-flags=.eeprom=alloc,load
            --change-section-lma .eeprom=0
            --no-change-warnings
            --output-target binary
            ${NAME}.elf ${eep_file}
            COMMAND ${AVR_SIZE} --mcu=${MCU} --format=avr ${NAME}.elf
    )

    set_property(
            DIRECTORY
            APPEND
            PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
            "${eep_file}"
            "${map_file}"
            "${bin_file}"
            "${lst_file}"
    )
endfunction(add_avr_executable)

function(add_mcu_executable NAME MCU)
    if (${MCU} MATCHES "atmega")
        add_avr_executable(${NAME} ${MCU} ${ARGN})
    else ()
        add_executable(${NAME}.elf apps/_empty.cpp)
    endif ()
endfunction(add_mcu_executable)

function(add_host_executable NAME)
    add_executable(${NAME} apps/_empty.cpp)
endfunction(add_host_executable)
