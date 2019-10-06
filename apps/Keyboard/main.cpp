#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include <zoal/board/arduino_leonardo.hpp>
#include <zoal/utils/tool_set.hpp>
#include <zoal/utils/ms_counter.hpp>
#include <zoal/utils/logger.hpp>
#include <zoal/io/button.hpp>
#include <zoal/io/analog_keypad.hpp>
#include <zoal/shield/uno_lcd.hpp>
#include <LUFA/Drivers/USB/USB.h>
#include <zoal/periph/rx_null_buffer.hpp>
#include "Keyboard.h"

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
tools::function_scheduler<16> scheduler;

uint16_t eeprom_buttons[shield::button_count] __attribute__((section (".eeprom"))) = {637, 411, 258, 101, 0};
uint8_t hid_report_buffer[sizeof(USB_KeyboardReport_Data_t)];

volatile bool press = true;
volatile uint8_t updateADC = 0;
volatile uint16_t adc_value = 0xFFFF;
const char *msg = "";
uint16_t prevADC = 0xFFFF;


USB_ClassInfo_HID_Device_t Keyboard_HID_Interface;

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
    scheduler.schedule(0, display_message);
}

void run() {
    using namespace zoal::io;
    using namespace zoal::gpio;

    if (updateADC) {
        updateADC = 0;
//        if (prevADC != adcValue) {
//            prevADC = adcValue;
//            shield::lcd::move(0, 0);
//            stream << adcValue << "     ";
//        }
        shield::handle_keypad(button_handler, adc_value);
    }

    scheduler.handle();
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

    eeprom_read_block(keypad::values, eeprom_buttons, sizeof(keypad::values));
    shield::gpio_cfg();
    shield::init();
    shield::calibrate(false);
    eeprom_write_block(keypad::values, eeprom_buttons, sizeof(keypad::values));

    mcu::mux::adc<adc, shield::analog_pin>::on();
    shield::adc::enable_interrupt();
    shield::adc::start();

    for (;;) {
        run();
    }
}

ISR(ADC_vect) {
    adc_value = shield::adc::value();
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

void EVENT_USB_Device_Connect(void) {
    msg = "Connected";
    scheduler.schedule(0, display_message);
}

void EVENT_USB_Device_Disconnect(void) {
    msg = "Disconnect";
    scheduler.schedule(0, display_message);
}

void EVENT_USB_Device_ConfigurationChanged(void) {
    bool ConfigSuccess = true;

    ConfigSuccess &= HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface);
    USB_Device_EnableSOFEvents();

    if (ConfigSuccess) {
        msg = "Config Success";
        scheduler.schedule(0, display_message);
    } else {
        msg = "Config Failed ";
        scheduler.schedule(0, display_message);
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
        KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_A;
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
    scheduler.schedule(0, display_message);
}
