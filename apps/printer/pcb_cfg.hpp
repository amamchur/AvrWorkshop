#ifndef AVR_WORKSHOP_PCB_CFG_HPP
#define AVR_WORKSHOP_PCB_CFG_HPP

#include <zoal/board/arduino_leonardo.hpp>
#include <zoal/shield/uno_lcd.hpp>
#include <zoal/utils/ms_counter.hpp>
#include <zoal/utils/tool_set.hpp>

#include "lcd_screen.hpp"
#include "printer/adpt1.hpp"
#include "printer/xlp504.hpp"

extern volatile uint32_t timer0_millis;
extern volatile uint16_t adc_value;

using mcu = zoal::pcb::mcu;
using adc = mcu::adc_00;
using timer = zoal::pcb::mcu::timer_00;
using ms_counter = zoal::utils::ms_counter<decltype(timer0_millis), &timer0_millis>;
using irq_handler = ms_counter::handler<zoal::pcb::mcu::frequency, 64, timer>;
using usart = mcu::usart_01;
using tx_buffer = usart::default_tx_buffer<16>;
//using rx_buffer = usart::default_rx_buffer<16>;
using rx_buffer = usart::null_rx_buffer;

using irq_handler = ms_counter::handler<zoal::pcb::mcu::frequency, 64, timer>;
using logger = zoal::utils::plain_logger<tx_buffer, zoal::utils::log_level::info>;
using tools = zoal::utils::tool_set<zoal::pcb::mcu, ms_counter, logger>;
using shield = zoal::shield::uno_lcd<tools, zoal::pcb, mcu::adc_00>;
using lcd = shield::lcd;
using keypad = shield::keypad;
using screen = lcd_screen<lcd>;

enum class printer_device {
    adtp1,
    xlp504,

    count
};

template <class T>
T shift_enum(T c, int d, T total) {
    return static_cast<T>(((int)c + (int)total + d) % (int)total);
}

extern printer_device current_printer_id;
extern uint32_t serial_number;
extern wchar_t serial_number_str[];
extern size_t serial_number_size;
extern tools::function_scheduler<8> scheduler;

constexpr auto printer_buffer_size = zoal::ct::max_type_size<printer::adpt1, printer::xlp504>::value;
extern uint8_t printer_buffer[printer_buffer_size];
extern printer::base *current_printer;

void read_eeprom_data();
void write_eeprom_data();
void format_serial_number();

void init_printer();
void deinit_printer();
void handle_usb();

#endif
