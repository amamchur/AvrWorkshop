#include "pcb_cfg.h"
#include <avr/eeprom.h>

volatile uint32_t timer0_millis = 0;
volatile uint16_t adc_value = 0xFFFF;
volatile bool running = true;

printer_device current_printer_id;
uint32_t serial_number = 0;
wchar_t serial_number_str[] = L"0000000000";
size_t serial_number_size = sizeof(serial_number_str);
uint8_t printer_buffer[printer_buffer_size];
printer::base *current_printer = nullptr;

tools::function_scheduler<16> timeout;

uint8_t eeprom_buttons[sizeof(keypad::values)] __attribute__((section(".eeprom")));
uint8_t eeprom_printer_id[sizeof(current_printer_id)] __attribute__((section(".eeprom")));
uint8_t eeprom_sn[sizeof(serial_number)] __attribute__((section(".eeprom")));

void read_eeprom_data() {
    eeprom_read_block(keypad::values, eeprom_buttons, sizeof(keypad::values));
    eeprom_read_block(&serial_number, eeprom_sn, sizeof(serial_number));
    eeprom_read_block(&current_printer_id, eeprom_printer_id, sizeof(current_printer_id));

    format_serial_number();

    current_printer_id = shift_enum(current_printer_id, 0, printer_device::count);
}

void write_eeprom_data() {
    eeprom_write_block(keypad::values, eeprom_buttons, sizeof(keypad::values));
    eeprom_write_block(&serial_number, eeprom_sn, sizeof(serial_number));
    eeprom_write_block(&current_printer_id, eeprom_printer_id, sizeof(current_printer_id));
}

void format_serial_number() {
    auto ptr = serial_number_str + sizeof(serial_number_str) / sizeof(serial_number_str[0]) - 1;
    auto v = serial_number;
    for (*ptr-- = 0x00; ptr >= serial_number_str; ptr--) {
        *ptr = static_cast<wchar_t >('0' + v % 10);
        v /= 10;
    }
}

void inc_serial_number(int d) {
    serial_number += d;

    format_serial_number();
}

void init_printer() {
    printer::base::shared_memory = printer_buffer;
    delete current_printer;
    switch (current_printer_id) {
        case printer_device::adtp1:
            current_printer = new printer::adpt1();
            break;
        default:
            current_printer = new printer::xlp504();
            break;
    }

    current_printer->init();
    USB_Init();
}

void deinit_printer() {
    USB_Disable();
}