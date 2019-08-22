#include "printer.h"

#include <LUFA/Drivers/USB/USB.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <zoal/board/arduino_leonardo.hpp>
#include <zoal/io/analog_keypad.hpp>
#include <zoal/io/button.hpp>
#include <zoal/periph/rx_null_buffer.hpp>
#include <zoal/shield/uno_lcd.hpp>
#include <zoal/utils/logger.hpp>
#include <zoal/utils/ms_counter.hpp>
#include <zoal/utils/tool_set.hpp>

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
tools::function_scheduler<16> timeout;

uint16_t button_values[shield::button_count] __attribute__((section(".eeprom"))) = {637, 411, 258, 101, 0};

volatile bool press = true;
volatile uint8_t updateADC = 0;
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
        press = true;
        break;
    case shield::left_btn:
        break;
    case shield::down_btn:
        break;
    case shield::up_btn:
        break;
    case shield::right_btn:
        break;
    }

    qwert[6] = '0' + button;

    msg = qwert;
    timeout.schedule(0, display_message);
}

void run() {
    using namespace zoal::io;
    using namespace zoal::gpio;

    if (updateADC) {
        updateADC = 0;
        shield::handle_keypad(button_handler, adcValue);
    }

    timeout.handle();

    uint8_t BytesReceived = PRNT_Device_BytesReceived(&PrinterInterface);
    bool update = BytesReceived > 0;
    int index = 0;
    for (; BytesReceived > 0; BytesReceived--, index++) {
        int16_t ReceivedByte = PRNT_Device_ReceiveByte(&PrinterInterface);
        qwert[index] = ReceivedByte;
    }

    if (update) {
        qwert[index] = 0;
        msg = qwert;
        timeout.schedule(0, display_message);
    }

    PRNT_Device_USBTask(&PrinterInterface);
    USB_USBTask();
}

char IEEE1284String[] = "";

int main() {
    using namespace zoal::io;
    using namespace zoal::gpio;

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
    shield::adc::start();
    updateADC = 1;
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

extern "C" void EVENT_USB_Device_Disconnect(void) {}

extern "C" void EVENT_USB_Device_ConfigurationChanged(void) {
    bool ConfigSuccess = true;

    ConfigSuccess &= PRNT_Device_ConfigureEndpoints(&PrinterInterface);
    if (ConfigSuccess) {
        msg = "Config Success";
        timeout.schedule(0, display_message);
    } else {
        msg = "Config Failed ";
        timeout.schedule(0, display_message);
    }
}

extern "C" void EVENT_USB_Device_ControlRequest(void) {
    PRNT_Device_ProcessControlRequest(&PrinterInterface);
}
