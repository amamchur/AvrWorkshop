#include <avr/eeprom.h>

#include <MCURDK/Board/ArduinoLeonardo.hpp>
#include <MCURDK/GPIO/SoftwareSPI.hpp>
#include <MCURDK/Utils/ToolSet.hpp>
#include <MCURDK/Utils/MillisecondsCounter.hpp>
#include <MCURDK/Utils/Prescalers.hpp>
#include <MCURDK/Utils/Logger.hpp>
#include <MCURDK/IC/MAX72xx.hpp>
#include <MCURDK/IO/Button.hpp>
#include <MCURDK/IO/AnalogKeypad.hpp>
#include <MCURDK/Shields/UnoLCDShield.hpp>

#include "Keyboard.h"

volatile uint32_t timer0_millis = 0;

typedef MCURDK::Utils::MillisecondsCounter<decltype(timer0_millis), &timer0_millis> Counter;
typedef MCURDK::PCB::Timer0 MsTimer;
typedef MCURDK::Utils::PrescalerLE<MsTimer, 64>::Result MsPrescaler;
typedef Counter::IRQHandler<MCURDK::PCB::MCU::Frequency, MsPrescaler::Value, MsTimer> IRQHandler;

typedef MCURDK::Periph::USARTConfig<MCURDK::PCB::MCU::Frequency, 115200> USARTCfg;
typedef MCURDK::PCB::MCU::HardwareUSART0<32, 8> HardwareUSART;

typedef MCURDK::Utils::PlainLogger<HardwareUSART, MCURDK::Utils::LogLevel::Info> Logger;
typedef MCURDK::Utils::ToolSet<MCURDK::PCB::MCU, Counter, Logger> Tools;

//typedef MCURDK::Shields::UnoLCDShieldConfig<MCURDK::PCB::BA05, MCURDK::PCB::BA04> SC;
typedef MCURDK::Shields::UnoLCDShield<Tools, MCURDK::PCB> Shield;
typedef Shield::Keypad Keypad;
typedef MCURDK::IO::OutputStream<Shield::LCD> LCDStream;
LCDStream stream;
Tools::FunctionTimeout<16> timeout;

typedef Tools::Delay Delay;

uint16_t ButtonValues[Shield::ButtonCount] __attribute__((section (".eeprom"))) = {637, 411, 258, 101, 0};
uint8_t PrevKeyboardHIDReportBuffer[sizeof(USB_KeyboardReport_Data_t)];

volatile bool press = true;
volatile uint8_t updateADC = 0;
volatile uint16_t adcValue = 0xFFFF;
const char *msg = "";
uint16_t prevADC = 0xFFFF;


USB_ClassInfo_HID_Device_t Keyboard_HID_Interface = {
        .Config = {
                .InterfaceNumber              = INTERFACE_ID_Keyboard,
                .ReportINEndpoint             = {
                        .Address              = KEYBOARD_EPADDR,
                        .Size                 = KEYBOARD_EPSIZE,
                        .Banks                = 1,
                },
                .PrevReportINBuffer           = PrevKeyboardHIDReportBuffer,
                .PrevReportINBufferSize       = sizeof(PrevKeyboardHIDReportBuffer),
        },
};

void displayMsg() {
    Shield::LCD::clear();
    stream << msg;
}

void initializeHardware() {
    HardwareUSART::begin<USARTCfg>();
    MsTimer::reset();
    MsTimer::mode<MCURDK::Periph::TimerMode::FastPWM8bit>();
    MsTimer::selectClockSource<MsPrescaler>();
    MsTimer::enableOverflowInterrupt();
    MCURDK::Utils::Interrupts::on();
}

void buttonHandler(size_t button, MCURDK::IO::ButtonEvent event) {
    if (event != MCURDK::IO::ButtonEventPress) {
        return;
    }

    switch (button) {
        case 0:
            press = true;
            break;
        case 1:
            msg = "";
            timeout.schedule(0, displayMsg);
            break;
        default:
            break;
    }
}

void run() {
    using namespace MCURDK::IO;
    using namespace MCURDK::GPIO;

    if (updateADC) {
        updateADC = 0;
//        if (prevADC != adcValue) {
//            prevADC = adcValue;
//            stream << pos(0, 0) << adcValue << "     ";
//        }
        Shield::handleKeypad(buttonHandler, adcValue);
    }

    timeout.handle();
    HID_Device_USBTask(&Keyboard_HID_Interface);
    USB_USBTask();
}

int main() {
    using namespace MCURDK::IO;
    using namespace MCURDK::GPIO;
    SetupHardware();
    initializeHardware();

    eeprom_read_block(Keypad::values, ButtonValues, sizeof(Keypad::values));
    Shield::init();
    Shield::calibrate(false);
    eeprom_write_block(Keypad::values, ButtonValues, sizeof(Keypad::values));

    Logger::clear();
    Logger::info() << "----- Started -----";

    Shield::AnalogChannel::on();
    ADCSRA |= 0x08;
    MCURDK::PCB::A2DC0::start();

    for (;;) {
        run();
    }
}

ISR(ADC_vect) {
    adcValue = MCURDK::PCB::A2DC0::value();
    updateADC = 1;

    Shield::AnalogChannel::on();
    MCURDK::PCB::A2DC0::start();
}

ISR(TIMER0_OVF_vect) {
    IRQHandler::handleTimerOverflow();
}

ISR(USART1_RX_vect) {
    HardwareUSART::handleRxIrq();
}

ISR(USART1_UDRE_vect) {
    HardwareUSART::handleTxIrq();
}

void SetupHardware() {
    clock_prescale_set(clock_div_1);
    USB_Init();
}

void EVENT_USB_Device_Connect(void) {
    msg = "Connected1";
    timeout.schedule(0, displayMsg);
}

void EVENT_USB_Device_Disconnect(void) {
    msg = "Disconnect";
    timeout.schedule(0, displayMsg);
}

void EVENT_USB_Device_ConfigurationChanged(void) {
    bool ConfigSuccess = true;

    ConfigSuccess &= HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface);
    USB_Device_EnableSOFEvents();

    if (ConfigSuccess) {
        msg = "Config Success";
        timeout.schedule(0, displayMsg);
    } else {
        msg = "Config Failed ";
        timeout.schedule(0, displayMsg);
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
    timeout.schedule(0, displayMsg);
}
