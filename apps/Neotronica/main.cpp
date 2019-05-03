#include <zoal/board/arduino_uno.hpp>
#include <zoal/ic/ds3231.hpp>
#include <zoal/ic/max72xx.hpp>
#include <zoal/ic/ws2812.hpp>
#include <zoal/io/button.hpp>
#include <zoal/io/ir_remote_receiver.hpp>
#include <zoal/periph/rx_null_buffer.hpp>
#include <zoal/periph/tx_ring_buffer.hpp>
#include <zoal/utils/logger.hpp>
#include <zoal/utils/ms_counter.hpp>
#include <zoal/utils/tool_set.hpp>

#include "matrix_helpers.hpp"

volatile uint32_t timer0_millis = 0;
const constexpr uint8_t device_count = 4;

using mcu = zoal::pcb::mcu;
using timer = zoal::pcb::mcu::timer_00;
using ms_counter = zoal::utils::ms_counter<decltype(timer0_millis), &timer0_millis>;
using irq_handler = ms_counter::handler<zoal::pcb::mcu::frequency, 64, timer>;
using usart = mcu::usart_00;
using usart_01_tx_buffer = zoal::periph::tx_ring_buffer<usart, 64>;
using usart_01_rx_buffer = zoal::periph::rx_null_buffer;

using i2c = mcu::i2c_00;
using logger = zoal::utils::terminal_logger<usart_01_tx_buffer, zoal::utils::log_level::trace>;
using tools = zoal::utils::tool_set<zoal::pcb::mcu, ms_counter, logger>;

using rtc_type = zoal::ic::ds3231<>;
using matrix_type = zoal::ic::max72xx_data<device_count>;
using max7219 = zoal::ic::max72xx<zoal::pcb::mcu::spi_00, zoal::pcb::ard_d10>;

using scheduler = tools::function_scheduler<16, int8_t>;

template<class Pin>
using button = typename zoal::io::button<tools, Pin, zoal::io::active_low_no_press>;

using i2c_stream = zoal::periph::i2c_stream<i2c>;
uint8_t i2c_buffer[sizeof(i2c_stream) + 64];
auto iic_stream = i2c_stream::from_memory(i2c_buffer, sizeof(i2c_buffer));

button<zoal::pcb::ard_a00> button1;
button<zoal::pcb::ard_d02> button2;
button<zoal::pcb::ard_d03> button3;
button<zoal::pcb::ard_d04> button4;
button<zoal::pcb::ard_d05> button5;
button<zoal::pcb::ard_d06> button6;
button<zoal::pcb::ard_d07> button7;
button<zoal::pcb::ard_d08> button8;
button<zoal::pcb::ard_d09> button9;
zoal::io::ir_remote_receiver<zoal::pcb::ard_a02, 25> receiver;

using transmitter = zoal::ic::transmitter_ws2812<zoal::pcb::ard_a01, F_CPU>;
using neo_pixel = zoal::ic::ws2812<transmitter, typename tools::delay>;

const constexpr uint8_t pixel_count = 8;
typedef struct pixel {
    uint8_t g;
    uint8_t r;
    uint8_t b;
} pixel;

pixel pixels[pixel_count];

const uint64_t digits[] = {
        0x3844444444444438, 0x3810101010103010, 0x7c20100804044438,
        0x3844040418044438, 0x04047c4424140c04, 0x384404047840407c,
        0x3844444478404438, 0x202020100804047c, 0x3844444438444438,
        0x3844043c44444438,
};

uint8_t current_data[device_count] = {0, 0, 0, 0};
uint8_t next_data[device_count];
uint16_t date_time = 0;
bool display_dots = false;
bool power_on = true;

const uint32_t ScrollTimeout = 60;
const int8_t ScrollSpace = 1;

scheduler timeout;
rtc_type rtc;
matrix_type matrix;
matrix_type buffer;
max7219::Command intensity = max7219::intensity2;
zoal::data::date_time currentDateTime;

void displayMinutesSeconds(int8_t);

void displayHoursMinutes(int8_t);

void (*displayFn)(int8_t) = &displayHoursMinutes;

void displayDayOfWeek() {
    memset(pixels, 0, sizeof(pixels));

    auto day = rtc[rtc_type::register_address::day];
    uint8_t ledOn[] = {0, 0x08, 0x14, 0x2A, 0x36, 0x55, 0x77, 0x7F};
    uint8_t mask = ledOn[day];
    for (uint8_t i = 0; power_on && i < pixel_count; i++) {
        if ((mask & (1u << i)) != 0) {
            pixels[i].b = 1;
        }
    }

    neo_pixel::send(pixels, pixel_count);
}

void fetchDateTime(int8_t) {
    rtc.fetch(iic_stream);
    displayDayOfWeek();
    timeout.schedule(1, displayFn, 0);
}

template<class T>
T shiftRight(T value, uint8_t, uint8_t, matrix_type &) {
    return static_cast<T>(value >> 1u);
}

void fillMatrix(matrix_type &m, const uint8_t *data) {
    for (int i = 0; i < device_count; i++) {
        m.set_rows(i, &digits[data[i]]);
    }

    m.transform_row(1, &shiftRight<uint8_t>);
    m.transform_row(3, &shiftRight<uint8_t>);
}

void initialize() {
    mcu::power<usart, timer, i2c>::on();

    mcu::mux::usart<usart, mcu::pd_00, mcu::pd_01, mcu::pd_04>::on();
    mcu::cfg::usart<usart, 115200>::apply();

    mcu::cfg::timer<timer, zoal::periph::timer_mode::up, 64, 1, 0xFF>::apply();
    mcu::irq::timer<timer>::enable_overflow_interrupt();

    mcu::mux::i2c<i2c, mcu::pc_04, mcu::pc_05>::on();
    mcu::cfg::i2c<i2c>::apply();

    mcu::enable<usart, timer, i2c>::on();

    zoal::utils::interrupts::on();

    memset(pixels, 0, sizeof(pixels));
    neo_pixel::begin();
    neo_pixel::send(pixels, pixel_count);

//    button1.begin();
//    button2.begin();
//    button3.begin();
//    button4.begin();
//    button5.begin();
//    button6.begin();
//    button7.begin();
//    button8.begin();
//    button9.begin();
    receiver.begin();

    matrix.clear();
    fillMatrix(matrix, current_data);

    max7219::init(device_count);
    max7219::send(device_count, intensity);
    max7219::display(matrix);

    TCCR2A = 2;
    TCCR2B = 2;
    OCR2A = 64;
    OCR2B = 0;
    ICR1 = 30;
    TIMSK2 = 1u << OCIE2A;
}

void powerOnOffHandler(zoal::io::button_event event) {
    if (event != zoal::io::button_event::press) {
        return;
    }

    power_on = !power_on;
    displayDayOfWeek();

    if (power_on) {
        max7219::send(device_count, max7219::on);
        timeout.schedule(0, &fetchDateTime, 0);
    } else {
        timeout.clear();
        max7219::send(device_count, max7219::off);
    }
}

void displayModeHandler(zoal::io::button_event event) {
    if (event != zoal::io::button_event::press) {
        return;
    }

    if (displayFn == &displayHoursMinutes) {
        displayFn = &displayMinutesSeconds;
    } else {
        displayFn = &displayHoursMinutes;
    }
}

void increaseIntensityHandler(zoal::io::button_event event) {
    if (event != zoal::io::button_event::press) {
        return;
    }

    if (intensity < max7219::intensityF) {
        intensity = (max7219::Command)((uint16_t) intensity + 1);
    }

    max7219::send(device_count, intensity);
}

void decreaseIntensityHandler(zoal::io::button_event event) {
    if (event != zoal::io::button_event::press) {
        return;
    }

    if (intensity > max7219::intensity0) {
        intensity = (max7219::Command)((uint16_t) intensity - 1);
    }

    max7219::send(device_count, intensity);
}

void buttonHandlerNoop(zoal::io::button_event event) {}

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

    for (uint8_t i = 0; i < device_count; i++) {
        if (current_data[i] != next_data[i]) {
            uint8_t r = buffer.insert_row(i, 0);
            matrix.insert_row(i, r);
        }
    }
    updateDots(display_dots);
    max7219::display(matrix);
}

void splitData(uint16_t v, uint8_t *data) {
    for (int i = 0; i < device_count; i++) {
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

    for (uint8_t i = 0; i < device_count; i++) {
        if (current_data[i] != next_data[i]) {
            matrix.insert_row(i, 0);
        }
    }
    updateDots(display_dots);
    max7219::display(matrix);
}

void displayMinutesSeconds(int8_t) {
    if (!rtc.ready()) {
        timeout.schedule(1, &displayMinutesSeconds, 0);
        return;
    }

    auto dt = rtc.date_time();
    uint16_t value = dt.minutes;
    value *= 100;
    value += dt.seconds;
    display_dots = true;

    splitData(date_time, current_data);
    splitData(value, next_data);
    fillMatrix(matrix, current_data);
    fillMatrix(buffer, next_data);

    max7219::display(matrix);
    timeout.schedule(0, &shiftScreen, ScrollSpace);
    timeout.schedule(1000, &fetchDateTime, 0);

    date_time = value;
    currentDateTime = dt;
}

void displayHoursMinutes(int8_t) {
    if (!rtc.ready()) {
        timeout.schedule(1, &displayHoursMinutes, 0);
        return;
    }

    auto dt = rtc.date_time();
    uint16_t value = dt.hours;
    value *= 100;
    value += dt.minutes;
    display_dots = (dt.seconds & 1u) == 0;

    splitData(date_time, current_data);
    splitData(value, next_data);
    fillMatrix(matrix, current_data);
    fillMatrix(buffer, next_data);

    max7219::display(matrix);
    timeout.schedule(0, &shiftScreen, ScrollSpace);
    timeout.schedule(1000, &fetchDateTime, 0);

    date_time = value;
    currentDateTime = dt;
}

void logDateTime(int8_t) {
    logger::info() << "Time: " << currentDateTime.year << "-"
                   << currentDateTime.month << "-" << currentDateTime.date << " "
                   << currentDateTime.hours << ":" << currentDateTime.minutes
                   << ":" << currentDateTime.seconds
                   << " day: " << currentDateTime.day;

    timeout.schedule(1000, &logDateTime, 0);
}

void handleIR(uint32_t code) {
    logger::info() << "IR code: " << zoal::io::hex << code;
    switch (code) {
        case 0x807F807F:
            powerOnOffHandler(zoal::io::button_event::press);
            break;
        case 0x807F827D:
            increaseIntensityHandler(zoal::io::button_event::press);
            break;
        case 0x807F42BD:
            decreaseIntensityHandler(zoal::io::button_event::press);
            break;
        case 0x807FA857:
            displayModeHandler(zoal::io::button_event::press);
        default:
            break;
    }
}

int main() {
    initialize();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

    logger::info() << "----- Started ------";

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

ISR(TIMER0_OVF_vect) {
    irq_handler::increment();
}

ISR(USART_RX_vect) {
    usart::rx_handler<usart_01_rx_buffer>();
}

ISR(USART_UDRE_vect) {
    usart::tx_handler<usart_01_tx_buffer>();
}

ISR(TWI_vect) {
    i2c::handle_irq();
}

ISR(TIMER2_COMPA_vect) {
    receiver.handle();
}
