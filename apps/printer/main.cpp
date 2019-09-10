#include "printer.h"

#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <zoal/board/arduino_leonardo.hpp>
#include <zoal/io/analog_keypad.hpp>
#include <zoal/periph/rx_null_buffer.hpp>
#include <zoal/shield/uno_lcd.hpp>
#include <zoal/utils/logger.hpp>
#include <zoal/utils/ms_counter.hpp>
#include <zoal/utils/tool_set.hpp>
#include "parser.hpp"

volatile uint32_t timer0_millis = 0;

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
using keypad = shield::keypad;

zoal::io::output_stream stream(zoal::io::transport_proxy<shield::lcd>::instance());
tools::function_scheduler<8> timeout;
parser<128> mpcl_parser;

uint16_t button_values[shield::button_count] __attribute__((section(".eeprom"))) = {637, 411, 258, 101, 0};

volatile uint16_t adcValue = 0xFFFF;
const char *msg = "";

USB_ClassInfo_PRNT_Device_t PrinterInterface;

void display_message() {
    shield::lcd::clear();
    stream << msg;
}

void initialize_hardware() {
    mcu::power<usart, timer>::on();

    mcu::mux::usart<usart, zoal::pcb::ard_d00, zoal::pcb::ard_d01, mcu::pd_05>::on();
    mcu::cfg::usart<usart, 115200>::apply();

    mcu::cfg::timer<timer, zoal::periph::timer_mode::up, 64, 1, 0xFF>::apply();
    mcu::irq::timer<timer>::enable_overflow_interrupt();

    mcu::cfg::adc<adc>::apply();

    mcu::enable<usart, timer, adc>::on();

    zoal::utils::interrupts::on();
}

char qwert[12] = "Press   ";

void button_handler(size_t button, zoal::io::button_event event) {
    if (event != zoal::io::button_event::press) {
        return;
    }

    switch (button) {
        case shield::select_btn:
            break;
        case shield::left_btn:
            break;
        case shield::down_btn:
            break;
        case shield::up_btn:
            break;
        case shield::right_btn:
            break;
        default:
            break;
    }

    qwert[6] = static_cast<char>('0' + button);

    msg = qwert;
    timeout.schedule(0, display_message);
}

void run() {
    using namespace zoal::io;
    using namespace zoal::gpio;

    if (adcValue != 0xFFFF) {
        zoal::utils::interrupts interrupts(false);
        shield::handle_keypad(button_handler, adcValue);
        adcValue = 0xFFFF;
        shield::adc::start();
    }

    timeout.handle();

    uint8_t received = PRNT_Device_BytesReceived(&PrinterInterface);
    for (; received > 0; received--) {
        int16_t byte = PRNT_Device_ReceiveByte(&PrinterInterface);
        mpcl_parser.push(static_cast<char>(byte));
    }

    PRNT_Device_USBTask(&PrinterInterface);
    USB_USBTask();
}

char IEEE1284String[] = "";

static void command_callback(base_parser *p, parse_event e) {
    const char *response = nullptr;
    switch (e) {
        case parse_event::command_enq:
            response = "\x05""A@\x06";
            msg = "ENQ Request ";
            timeout.schedule(0, display_message);
            break;
        case parse_event::command_mm:
            response = "M46\x06";
            break;
        case parse_event::command_mv:
            response = "V03\x06";
            break;
        case parse_event::command_mr:
            response = "R04\x06";
            break;
        case parse_event::command_mc:
            response = "C00\x06";
            break;
        case parse_event::command_mi:
            response = "I00\x06";
            break;
        case parse_event::command_mp:
            response = "P00\x06";
            break;
        case parse_event::command_mts:
            response = "MTS18088753\x06";
            break;
        default:
            return;
    }

    if (response != nullptr) {
        PRNT_Device_SendString(&PrinterInterface, response);
    }
}

int main() {
    using namespace zoal::io;
    using namespace zoal::gpio;

    mpcl_parser.callback(command_callback);

    PrinterInterface.Config.InterfaceNumber = INTERFACE_ID_Printer;
    PrinterInterface.Config.DataINEndpoint.Address = PRINTER_IN_EPADDR;
    PrinterInterface.Config.DataINEndpoint.Size = PRINTER_IO_EPSIZE;
    PrinterInterface.Config.DataINEndpoint.Banks = 1;
    PrinterInterface.Config.DataOUTEndpoint.Address = PRINTER_OUT_EPADDR;
    PrinterInterface.Config.DataOUTEndpoint.Size = PRINTER_IO_EPSIZE;
    PrinterInterface.Config.DataOUTEndpoint.Banks = 1;
    PrinterInterface.Config.IEEE1284String = IEEE1284String;

    SetupHardware();

    initialize_hardware();

    eeprom_read_block(keypad::values, button_values, sizeof(keypad::values));
    shield::gpio_cfg();
    shield::init();
    shield::calibrate(false);
    eeprom_write_block(keypad::values, button_values, sizeof(keypad::values));

    mcu::mux::adc<adc, shield::analog_pin>::on();
    shield::adc::enable_interrupt();
    shield::adc::start();

    for (;;) {
        run();
    }
}

ISR(ADC_vect) {
    adcValue = shield::adc::value();
}

ISR(TIMER0_OVF_vect) {
    irq_handler::increment();
}

ISR(USART1_RX_vect) {
    usart::rx_handler_v2<rx_buffer>();
}

ISR(USART1_UDRE_vect) {
    usart::tx_handler_v2<tx_buffer>();
}

void SetupHardware() {
    clock_prescale_set(clock_div_1);
    USB_Init();
}

extern "C" void EVENT_USB_Device_Connect(void) {
    msg = "Connected";
    timeout.schedule(0, display_message);
}

extern "C" void EVENT_USB_Device_Disconnect() {}

extern "C" void EVENT_USB_Device_ConfigurationChanged() {
    bool ConfigSuccess = PRNT_Device_ConfigureEndpoints(&PrinterInterface);
    msg = ConfigSuccess ? "Config Success" : "Config Failed ";
    timeout.schedule(0, display_message);
}

extern "C" void EVENT_USB_Device_ControlRequest() {
    PRNT_Device_ProcessControlRequest(&PrinterInterface);
}
