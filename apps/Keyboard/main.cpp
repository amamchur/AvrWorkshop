#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include <zoal/board/arduino_leonardo.hpp>
#include <zoal/gpio/software_spi.hpp>
#include <zoal/utils/tool_set.hpp>
#include <zoal/utils/ms_counter.hpp>
#include <zoal/utils/prescalers.hpp>
#include <zoal/utils/logger.hpp>
#include <zoal/ic/max72xx.hpp>
#include <zoal/io/Button.hpp>
#include <zoal/io/analog_keypad.hpp>
#include <zoal/shields/uno_lcd_shield.hpp>
#include <LUFA/Drivers/USB/USB.h>

#include "Keyboard.h"

volatile uint32_t timer0_millis = 0;

using ms_counter = zoal::utils::ms_counter<uint32_t, &timer0_millis>;
using ms_timer = zoal::pcb::mcu::timer0;
using ms_prescaler = zoal::utils::prescaler_le<ms_timer, 64>::result;
using irq_handler = ms_counter::handler<zoal::pcb::mcu::frequency, ms_prescaler::value, ms_timer>;
using usart_config = zoal::periph::usart_config<zoal::pcb::mcu::frequency, 115200>;
using usart = zoal::pcb::mcu::usart0<32, 8>;
using logger = zoal::utils::plain_logger<usart, zoal::utils::log_level ::info>;
using tools = zoal::utils::tool_set<zoal::pcb::mcu, ms_counter, logger>;
using shield = zoal::shields::uno_lcd_shield<tools, zoal::pcb>;
using keypad = shield::keypad;
using lcd_stream = zoal::io::output_stream<shield::lcd>;

lcd_stream stream;
tools::function_scheduler<16> timeout;

uint16_t button_values[shield::button_count] __attribute__((section (".eeprom"))) = {637, 411, 258, 101, 0};
uint8_t hid_report_buffer[sizeof(USB_KeyboardReport_Data_t)];

volatile bool press = true;
volatile uint8_t updateADC = 0;
volatile uint16_t adcValue = 0xFFFF;
const char *msg = "";
uint16_t prevADC = 0xFFFF;


USB_ClassInfo_HID_Device_t Keyboard_HID_Interface;

void display_message() {
    shield::lcd::clear();
    stream << msg;
}

void initialize_hardware() {
    usart::setup<usart_config>();
    ms_timer::reset();
    ms_timer::mode<zoal::periph::timer_mode::fast_pwm_8bit>();
    ms_timer::select_clock_source<ms_prescaler>();
    ms_timer::enable_overflow_interrupt();
    zoal::utils::interrupts::on();
}

void button_handler(size_t button, zoal::io::button_event event) {
    if (event != zoal::io::button_event::press) {
        return;
    }

    switch (button) {
        case 0:
            press = true;
            break;
        case 1:
            msg = "";
            timeout.schedule(0, display_message);
            break;
        default:
            break;
    }
}

void run() {
    using namespace zoal::io;
    using namespace zoal::gpio;

    if (updateADC) {
        updateADC = 0;
//        if (prevADC != adcValue) {
//            prevADC = adcValue;
//            stream << pos(0, 0) << adcValue << "     ";
//        }
        shield::handle_keypad(button_handler, adcValue);
    }

    timeout.handle();
    HID_Device_USBTask(&Keyboard_HID_Interface);
    USB_USBTask();
}

int main() {
    using namespace zoal::io;
    using namespace zoal::gpio;

    Keyboard_HID_Interface.Config.InterfaceNumber = INTERFACE_ID_Keyboard;
    Keyboard_HID_Interface.Config.ReportINEndpoint.Address = KEYBOARD_EPADDR;
    Keyboard_HID_Interface.Config.ReportINEndpoint.Size = KEYBOARD_EPSIZE;
    Keyboard_HID_Interface.Config.ReportINEndpoint.Banks = 1;
    Keyboard_HID_Interface.Config.PrevReportINBuffer = hid_report_buffer;
    Keyboard_HID_Interface.Config.PrevReportINBufferSize = sizeof(hid_report_buffer);

    SetupHardware();
    initialize_hardware();

    eeprom_read_block(keypad::values, button_values, sizeof(keypad::values));
    shield::init();
    shield::calibrate(false);
    eeprom_write_block(keypad::values, button_values, sizeof(keypad::values));

    logger::clear();
    logger::info() << "----- Started -----";

    shield::analog_channel::on();
    ADCSRA |= 0x08;
    shield::analog_channel::adc::start();

    for (;;) {
        run();
    }
}

ISR(ADC_vect) {
    shield::analog_channel::adc::value();
    updateADC = 1;

    shield::analog_channel::on();
    shield::analog_channel::adc::start();
}

ISR(TIMER0_OVF_vect) {
    irq_handler::increment();
}

ISR(USART1_RX_vect) {
    usart::handle_rx_irq();
}

ISR(USART1_UDRE_vect) {
    usart::handle_tx_irq();
}

void SetupHardware() {
    clock_prescale_set(clock_div_1);
    USB_Init();
}

void EVENT_USB_Device_Connect(void) {
    msg = "Connected1";
    timeout.schedule(0, display_message);
}

void EVENT_USB_Device_Disconnect(void) {
    msg = "Disconnect";
    timeout.schedule(0, display_message);
}

void EVENT_USB_Device_ConfigurationChanged(void) {
    bool ConfigSuccess = true;

    ConfigSuccess &= HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface);
    USB_Device_EnableSOFEvents();

    if (ConfigSuccess) {
        msg = "Config Success";
        timeout.schedule(0, display_message);
    } else {
        msg = "Config Failed ";
        timeout.schedule(0, display_message);
    }
}

void EVENT_USB_Device_ControlRequest(void) {
    HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void) {
    HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
}

bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t *const HIDInterfaceInfo,
                                         uint8_t *const ReportID,
                                         const uint8_t ReportType,
                                         void *ReportData,
                                         uint16_t *const ReportSize) {
    auto *KeyboardReport = (USB_KeyboardReport_Data_t *) ReportData;
    memset(KeyboardReport->KeyCode, 0, sizeof(KeyboardReport->KeyCode));
    if (press) {
        KeyboardReport->KeyCode[0] = 4;
        press = false;
    }
    *ReportSize = sizeof(USB_KeyboardReport_Data_t);
    return false;
}

void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t *const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void *ReportData,
                                          const uint16_t ReportSize) {
    msg = "ProcessHID";
    timeout.schedule(0, display_message);
}
