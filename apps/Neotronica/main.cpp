#include <MCURDK/Board/ArduinoUno.hpp>
#include <MCURDK/Utils/ToolSet.hpp>
#include <MCURDK/Utils/MillisecondsCounter.hpp>
#include <MCURDK/Utils/Prescalers.hpp>
#include <MCURDK/Utils/Logger.hpp>
#include <MCURDK/IO/Button.hpp>
#include <MCURDK/IO/IRRemoteReceiver.hpp>
#include <MCURDK/IC/MAX72xx.hpp>
#include <MCURDK/IC/DS3231.hpp>
#include <MCURDK/IC/WS2812.hpp>

#include "MatrixHelpers.hpp"

volatile uint32_t timer0_millis = 0;
const constexpr uint8_t DeviceCount = 4;

typedef MCURDK::Periph::USARTConfig<MCURDK::PCB::MCU::Frequency, 115200> USARTCfg;
typedef MCURDK::Periph::I2CConfig<F_CPU> CfgI2C;
typedef MCURDK::PCB::MCU::HardwareUSART0<32, 8> HardwareUSART;
typedef MCURDK::PCB::MCU::HardwareMasterI2C<32> I2C;

typedef MCURDK::Utils::MillisecondsCounter<decltype(timer0_millis), &timer0_millis> Counter;
typedef MCURDK::PCB::Timer0 MsTimer;
typedef MCURDK::Utils::PrescalerLE<MsTimer, 64>::Result MsPrescaler;
typedef MCURDK::Utils::TerminalLogger<HardwareUSART, MCURDK::Utils::LogLevel::Trace> Logger;
typedef MCURDK::Utils::ToolSet<MCURDK::PCB::MCU, Counter, Logger> Tools;

typedef MCURDK::IC::DS3231<I2C> RTC;
typedef MCURDK::IC::MAX72xxData<DeviceCount> Matrix;
typedef MCURDK::IC::MAX72xx<Tools, MCURDK::PCB::HardwareSPI0, MCURDK::PCB::BD10> MAX7219;

typedef Tools::FunctionTimeout<16, int8_t> Timeout;

template<class Pin>
using Button = typename MCURDK::IO::Button<Pin, Counter, MCURDK::IO::PullUpButtonNoPress>;

Button<MCURDK::PCB::BA00> button1;
Button<MCURDK::PCB::BD02> button2;
Button<MCURDK::PCB::BD03> button3;
Button<MCURDK::PCB::BD04> button4;
Button<MCURDK::PCB::BD05> button5;
Button<MCURDK::PCB::BD06> button6;
Button<MCURDK::PCB::BD07> button7;
Button<MCURDK::PCB::BD08> button8;
Button<MCURDK::PCB::BD09> button9;
MCURDK::IO::IRRemoteReceiver<MCURDK::PCB::BA02, 25> receiver;

typedef MCURDK::IC::TransmitterWS2812<MCURDK::PCB::BA01, F_CPU> Transmitter;
typedef MCURDK::IC::WS2812<Transmitter, typename Tools::Delay> NeoPixel;
const constexpr uint8_t PixelsCount = 8;
typedef struct Pixel {
    uint8_t g;
    uint8_t r;
    uint8_t b;
} Pixel;

Pixel pixels[PixelsCount];

const uint64_t Digits[] = {
        0x3844444444444438,
        0x3810101010103010,
        0x7c20100804044438,
        0x3844040418044438,
        0x04047c4424140c04,
        0x384404047840407c,
        0x3844444478404438,
        0x202020100804047c,
        0x3844444438444438,
        0x3844043c44444438,
};

uint8_t currentData[DeviceCount] = {0, 0, 0, 0};
uint8_t nextData[DeviceCount];
uint16_t dateTime = 0;
bool displayDots = false;
bool powerOn = true;

const uint32_t ScrollTimeout = 60;
const int8_t ScrollSpace = 1;

Timeout timeout;
RTC rtc;
MAX7219 max7219;
Matrix matrix;
Matrix buffer;
MAX7219::Command intensity = MAX7219::Intensity2;
MCURDK::Data::DateTime currentDateTime;

void displayMinutesSeconds(int8_t);

void displayHoursMinutes(int8_t);

void (*displayFn)(int8_t) = &displayHoursMinutes;

void displayDayOfWeek() {
    memset(pixels, 0, sizeof(pixels));

    auto day = rtc[RTC::AddrDay];
    uint8_t ledOn[] = {0, 0x08, 0x14, 0x2A, 0x36, 0x55, 0x77, 0x7F};
    uint8_t mask = ledOn[day];
    for (uint8_t i = 0; powerOn && i < PixelsCount; i++) {
        if ((mask & (1u << i)) != 0) {
            pixels[i].b = 1;
        }
    }

    NeoPixel::send(pixels, PixelsCount);
}

void fetchDateTime(int8_t) {
    rtc.fetch();
    displayDayOfWeek();
    timeout.schedule(1, displayFn, 0);
}

template<class T>
T shiftRight(T value, uint8_t, uint8_t, Matrix &) {
    return static_cast<T>(value >> 1u);
}

void fillMatrix(Matrix &m, const uint8_t *data) {
    for (int i = 0; i < DeviceCount; i++) {
        m.setRows(i, &Digits[data[i]]);
    }

    m.transformRow(1, &shiftRight<uint8_t>);
    m.transformRow(3, &shiftRight<uint8_t>);
}

void initialize() {
    HardwareUSART::begin<USARTCfg>();
    I2C::begin<CfgI2C>();

    MsTimer::reset();
    MsTimer::mode<MCURDK::Periph::TimerMode::FastPWM8bit>();
    MsTimer::selectClockSource<MsPrescaler>();
    MsTimer::enableOverflowInterrupt();
    MCURDK::Utils::Interrupts::on();

    memset(pixels, 0, sizeof(pixels));
    NeoPixel::begin();
    NeoPixel::send(pixels, PixelsCount);

    button1.begin();
    button2.begin();
    button3.begin();
    button4.begin();
    button5.begin();
    button6.begin();
    button7.begin();
    button8.begin();
    button9.begin();
    receiver.begin();

    matrix.clear();
    fillMatrix(matrix, currentData);

    max7219.begin(DeviceCount);
    max7219(DeviceCount, intensity);
    max7219.display(matrix);

    TCCR2A = 2;
    TCCR2B = 2;
    OCR2A = 64;
    OCR2B = 0;
    ICR1 = 30;
    TIMSK2 = 1 << OCIE2A;
}

void powerOnOffHandler(MCURDK::IO::ButtonEvent event) {
    if (event != MCURDK::IO::ButtonEventPress) {
        return;
    }

    powerOn = !powerOn;
    displayDayOfWeek();

    if (powerOn) {
        max7219(DeviceCount, MAX7219::On);
        timeout.schedule(0, &fetchDateTime, 0);
    } else {
        timeout.clear();
        max7219(DeviceCount, MAX7219::Off);
    }
}

void displayModeHandler(MCURDK::IO::ButtonEvent event) {
    if (event != MCURDK::IO::ButtonEventPress) {
        return;
    }

    if (displayFn == &displayHoursMinutes) {
        displayFn = &displayMinutesSeconds;
    } else {
        displayFn = &displayHoursMinutes;
    }
}

void increaseIntensityHandler(MCURDK::IO::ButtonEvent event) {
    if (event != MCURDK::IO::ButtonEventPress) {
        return;
    }

    if (intensity < MAX7219::IntensityF) {
        intensity = (MAX7219::Command)((uint16_t) intensity + 1);
    }

    max7219(DeviceCount, intensity);
}

void decreaseIntensityHandler(MCURDK::IO::ButtonEvent event) {
    if (event != MCURDK::IO::ButtonEventPress) {
        return;
    }

    if (intensity > MAX7219::Intensity0) {
        intensity = (MAX7219::Command)((uint16_t) intensity - 1);
    }

    max7219(DeviceCount, intensity);
}


void buttonHandlerNoop(MCURDK::IO::ButtonEvent event) {
}

void handleButtons() {
    button1.handle(powerOnOffHandler);
    button2.handle(displayModeHandler);
    button3.handle(buttonHandlerNoop);
    button4.handle(increaseIntensityHandler);
    button5.handle(decreaseIntensityHandler);
    button6.handle(buttonHandlerNoop);
    button7.handle(buttonHandlerNoop);
    button8.handle(buttonHandlerNoop);
    button9.handle(buttonHandlerNoop);
}

void updateDots(bool visible) {
    for (int i = 0; i < 8; i++) {
        matrix.data[1][i] &= ~0x80u;
        matrix.data[2][i] &= ~0x01u;
    }

    if (visible) {
        matrix.data[1][1] |= 0x80u;
        matrix.data[1][2] |= 0x80u;
        matrix.data[1][5] |= 0x80u;
        matrix.data[1][6] |= 0x80u;

        matrix.data[2][1] |= 0x01u;
        matrix.data[2][2] |= 0x01u;
        matrix.data[2][5] |= 0x01u;
        matrix.data[2][6] |= 0x01u;
    }
}

void scrollScreen(int8_t step) {
    if (step > 0) {
        timeout.schedule(ScrollTimeout, &scrollScreen, step - 1);
    }

    for (uint8_t i = 0; i < DeviceCount; i++) {
        if (currentData[i] != nextData[i]) {
            uint8_t r = buffer.insertRow(i, 0);
            matrix.insertRow(i, r);
        }
    }
    updateDots(displayDots);
    max7219.display(matrix);
}

void splitData(uint16_t v, uint8_t *data) {
    for (int i = 0; i < DeviceCount; i++) {
        data[i] = static_cast<uint8_t>(v % 10);
        v /= 10;
    }
}

void shiftScreen(int8_t step) {
    if (step == 0) {
        timeout.schedule(ScrollTimeout, &scrollScreen, 7);
    } else {
        timeout.schedule(ScrollTimeout, &shiftScreen, step - 1);
    }

    for (uint8_t i = 0; i < DeviceCount; i++) {
        if (currentData[i] != nextData[i]) {
            matrix.insertRow(i, 0);
        }
    }
    updateDots(displayDots);
    max7219.display(matrix);
}


void displayMinutesSeconds(int8_t) {
    if (!rtc.ready) {
        timeout.schedule(1, &displayMinutesSeconds, 0);
        return;
    }

    auto dt = rtc.dateTime();
    uint16_t value = dt.minutes;
    value *= 100;
    value += dt.seconds;
    displayDots = true;

    splitData(dateTime, currentData);
    splitData(value, nextData);
    fillMatrix(matrix, currentData);
    fillMatrix(buffer, nextData);

    max7219.display(matrix);
    timeout.schedule(0, &shiftScreen, ScrollSpace);
    timeout.schedule(1000, &fetchDateTime, 0);

    dateTime = value;
    currentDateTime = dt;
}

void displayHoursMinutes(int8_t) {
    if (!rtc.ready) {
        timeout.schedule(1, &displayHoursMinutes, 0);
        return;
    }

    auto dt = rtc.dateTime();
    uint16_t value = dt.hours;
    value *= 100;
    value += dt.minutes;
    displayDots = (dt.seconds & 1u) == 0;

    splitData(dateTime, currentData);
    splitData(value, nextData);
    fillMatrix(matrix, currentData);
    fillMatrix(buffer, nextData);

    max7219.display(matrix);
    timeout.schedule(0, &shiftScreen, ScrollSpace);
    timeout.schedule(1000, &fetchDateTime, 0);

    dateTime = value;
    currentDateTime = dt;
}

void logDateTime(int8_t) {
    Logger::info() << "Time: "
                   << currentDateTime.year << "-"
                   << currentDateTime.month << "-"
                   << currentDateTime.date << " "
                   << currentDateTime.hours << ":"
                   << currentDateTime.minutes << ":"
                   << currentDateTime.seconds << " day: "
                   << currentDateTime.day;

    timeout.schedule(1000, &logDateTime, 0);
}

void handleIR(uint32_t code) {
    Logger::info() << "IR code: " << MCURDK::IO::hex << code;
    switch (code) {
        case 0x807F807F:
            powerOnOffHandler(MCURDK::IO::ButtonEvent::ButtonEventPress);
            break;
        case 0x807F827D:
            increaseIntensityHandler(MCURDK::IO::ButtonEvent::ButtonEventPress);
            break;
        case 0x807F42BD:
            decreaseIntensityHandler(MCURDK::IO::ButtonEvent::ButtonEventPress);
            break;
        case 0x807FA857:
            displayModeHandler(MCURDK::IO::ButtonEvent::ButtonEventPress);
        default:
            break;
    }
}

int main() {
    initialize();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

    Logger::info() << "----- Started ------";

//    rtc.fetch();
//    while (!rtc.ready);
//    currentDateTime = rtc.dateTime();
//    logDateTime(0);
//    currentDateTime.year = 2018;
//    currentDateTime.month = 7;
//    currentDateTime.date = 22;
//    currentDateTime.day = 7;
//    currentDateTime.hours = 20;
//    currentDateTime.minutes = 47;
//    currentDateTime.seconds = 0;
//    rtc.dateTime(currentDateTime);
//    rtc.update();
//    while (!rtc.ready);

    timeout.schedule(0, &fetchDateTime, 0);
    //    timeout.schedule(100, &logDateTime, 0);
    while (true) {
        handleButtons();
        timeout.handle();
        if (receiver.processed()) {
            auto code = receiver.result();
            handleIR(code);
            receiver.run();
        }
    }
    return 0;
#pragma clang diagnostic pop
}

typedef Counter::IRQHandler <MCURDK::PCB::MCU::Frequency, MsPrescaler::Value, MsTimer> IRQHandler;

ISR(TIMER0_OVF_vect) {
    IRQHandler::handleTimerOverflow();
}

ISR(TIMER1_OVF_vect) {
    IRQHandler::handleTimerOverflow();
}

ISR(USART_RX_vect) {
    HardwareUSART::handleRxIrq();
}

ISR(USART_UDRE_vect) {
    HardwareUSART::handleTxIrq();
}

ISR(TWI_vect) {
    I2C::handleIrq();
}

ISR (TIMER2_COMPA_vect) {
    receiver.handle();
}
