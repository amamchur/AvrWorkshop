#include <zoal/shield/uno_lcd.hpp>
#include "menu.h"

#include "pcb_cfg.h"
#include "lcd_screen.h"

namespace {
    size_t calibration_button;
    uint16_t max_adc_value;
    lcd_display_mode display_mode = lcd_display_mode::printer_info;
}

void start_keypad_calibration();

void start_serial_number_cfg();

void start_printer_select();

void display_serial_number();

void config_serial_number(size_t button, zoal::io::button_event event);

void config_current_printer(size_t button, zoal::io::button_event event);

void menu_input(size_t button, zoal::io::button_event event);

void (*keypad_handler)(size_t button, zoal::io::button_event event) = config_serial_number;

void update_display();

void read_adc_async() {
    shield::adc::wait();
    adc_value = 0xFFFF;
    shield::adc::start();
}

void config_serial_number(size_t button, zoal::io::button_event event) {
    if (event != zoal::io::button_event::press) {
        return;
    }

    switch (button) {
        case shield::select_btn:
            timeout.schedule(0, start_printer_select);
            return;
        case shield::down_btn:
            inc_serial_number(-1);
            break;
        case shield::up_btn:
            inc_serial_number(1);
            break;
        default:
            break;
    }

    display_serial_number();
}

void display_serial_number() {
    screen::clear();
    screen::print(0, 0, "Enter SN");
    screen::print(1, 0, serial_number_str);
    screen::flush();
}

void display_printer() {
    screen::clear();
    screen::print(0, 0, "Printer");
    switch (current_printer_id) {
        case printer_device::adtp1:
            screen::print(1, 0, "> ADTP1");
            break;
        case printer_device::xlp504:
            screen::print(1, 0, "> XLP504");
            break;
        default:
            break;
    }

    screen::flush();
}

static void create_printer() {
    init_printer();
    timeout.schedule(0, start_main_menu);
}

static void destroy_printer() {
    deinit_printer();
    timeout.schedule(500, create_printer);
}

void config_current_printer(size_t button, zoal::io::button_event event) {
    if (event != zoal::io::button_event::press) {
        return;
    }

    switch (button) {
        case shield::select_btn:
            write_eeprom_data();
            timeout.schedule(0, destroy_printer);
            return;
        case shield::down_btn:
            current_printer_id = shift_enum(current_printer_id, 1, printer_device::count);
            break;
        case shield::up_btn:
            current_printer_id = shift_enum(current_printer_id, -1, printer_device::count);
            break;
        default:
            return;
    }

    display_printer();
}

void menu_input(size_t button, zoal::io::button_event event) {
    if (event != zoal::io::button_event::press) {
        return;
    }

    switch (button) {
        case shield::down_btn:
            display_mode = shift_enum(display_mode, 1, lcd_display_mode::count);
            break;
        case shield::up_btn:
            display_mode = shift_enum(display_mode, -1, lcd_display_mode::count);
            break;
        default:
            return;
    }

    update_display();
}

void read_keypad_adc() {
    timeout.schedule(0, read_keypad_adc);

    auto value = adc_value;
    if (value == 0xFFFF) {
        return;
    }

    read_adc_async();
    shield::handle_keypad(keypad_handler, value);
}

void read_calibration_value() {
    if (adc_value == 0xFFFF) {
        timeout.schedule(0, read_calibration_value);
        return;
    }

    max_adc_value = max_adc_value > adc_value ? adc_value : max_adc_value;
    int diff = static_cast<int>(max_adc_value) - adc_value;
    if (diff < -50) {
        keypad::values[calibration_button] = max_adc_value;
        calibration_button++;
        start_keypad_calibration();
        return;
    }

    read_adc_async();
    timeout.schedule(0, read_calibration_value);
}

void start_serial_number_cfg() {
    display_serial_number();

    read_adc_async();

    keypad_handler = config_serial_number;
    timeout.schedule(0, read_keypad_adc);
}

void start_printer_select() {
    display_printer();
    read_adc_async();

    keypad_handler = config_current_printer;

    timeout.clear();
    timeout.schedule(0, read_keypad_adc);
}

void start_keypad_calibration() {
    const void *msg;
    bool complete = false;
    switch (calibration_button) {
        case shield::select_btn:
            msg = calibrate_select;
            break;
        case shield::left_btn:
            msg = calibrate_left;
            break;
        case shield::down_btn:
            msg = calibrate_down;
            break;
        case shield::up_btn:
            msg = calibrate_up;
            break;
        case shield::right_btn:
            msg = calibrate_right;
            break;
        default:
            msg = calibrate_complete;
            complete = true;
            break;
    }
    screen::copy_pgm(msg);
    screen::flush();

    if (complete) {
        timeout.schedule(1000, start_serial_number_cfg);
    } else {
        read_adc_async();
        max_adc_value = 1u << 12u;
        timeout.schedule(100, read_calibration_value);
    }
}

void start_hartware_configuration() {
    mcu::mux::adc<adc, shield::analog_pin>::on();

    screen::copy_pgm(menu_greeting);
    screen::flush();

    calibration_button = shield::select_btn;

    timeout.clear();
    timeout.schedule(3000, start_keypad_calibration);
}

void update_display() {
    screen::clear();

    switch (display_mode) {
        case lcd_display_mode::printer_info:
            screen::print(0, 0, current_printer->name());
            screen::print(1, 0, "SN: ");
            screen::print(1, 4, serial_number_str);
            break;
        case lcd_display_mode::rx_tx_info: {
            screen::print(0, 0, "RX:");
            screen::print_right(0, current_printer->rx_bytes());
            screen::print(1, 0, "TX:");
            screen::print_right(1, current_printer->tx_bytes());
            break;
        }
        default:
            break;
    }

    screen::flush();
}

void start_main_menu() {
    mcu::mux::adc<adc, shield::analog_pin>::on();
    update_display();

    keypad_handler = menu_input;

    timeout.clear();
    timeout.schedule(0, read_keypad_adc);
}
