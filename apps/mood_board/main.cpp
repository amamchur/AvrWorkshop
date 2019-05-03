#include "../../libs/jsonlite/jsonlite.h"

#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <zoal/arch/avr/atmega/i2c.hpp>
#include <zoal/arch/avr/atmega/spi.hpp>
#include <zoal/arch/avr/port.hpp>
#include <zoal/board/arduino_uno.hpp>
#include <zoal/data/rx_tx_buffer.hpp>
#include <zoal/ic/max72xx.hpp>
#include <zoal/io/input_stream.hpp>
#include <zoal/periph/rx_null_buffer.hpp>
#include <zoal/periph/rx_ring_buffer.hpp>
#include <zoal/periph/software_spi.hpp>
#include <zoal/periph/tx_ring_buffer.hpp>
#include <zoal/shield/uno_lcd.hpp>
#include <zoal/utils/helpers.hpp>
#include <zoal/utils/logger.hpp>
#include <zoal/utils/ms_counter.hpp>
#include <zoal/utils/tool_set.hpp>


template<class T, class U>
class io_ring_buffer {
public:
    bool put(uint8_t value) {
        zoal::utils::interrupts di(false);
        auto next = head + 1;
        if (next >= size) {
            next = 0;
        }

        while (next == tail) {
            zoal::utils::cooperation<>::yield();
        }

        buffer[head] = value;
        head = next;

        U::enable_tx();
        return true;
    }

    bool get(uint8_t &value) {
        zoal::utils::interrupts di(false);
        if (tail == head) {
            return false;
        }

        value = buffer[tail++];

        if (tail >= size) {
            tail = 0;
        }

        return true;
    }

    static constexpr size_t buffer_size = 2;

    T buffer[buffer_size];
    size_t size{buffer_size};
    volatile size_t head{0};
    volatile size_t tail{0};
};

volatile uint32_t milliseconds = 0;

using mcu = zoal::pcb::mcu;
using counter = zoal::utils::ms_counter<decltype(milliseconds), &milliseconds>;
using timer = mcu::timer_00;
using ir_timer = mcu::timer_02;
using adc = mcu::adc_00;
using spi = mcu::spi_00;
using irq_handler = counter::handler<mcu::frequency, 64, timer>;
using usart = mcu::usart_00;
using usart_01_tx_buffer = zoal::periph::tx_ring_buffer<usart, 64>;
using usart_01_rx_buffer = zoal::periph::rx_ring_buffer<usart, 64>;

using spi = mcu::spi_00;
using sspi = zoal::periph::tx_software_spi<mcu::pb_03, mcu::pb_02>;

using logger = zoal::utils::terminal_logger<usart_01_tx_buffer, zoal::utils::log_level::trace>;
using tools = zoal::utils::tool_set<mcu, counter, logger>;
using delay = tools::delay;
using matrix_type = zoal::ic::max72xx_data<4>;
using max7219 = zoal::ic::max72xx<spi, zoal::pcb::ard_d10>;
using scheduler_type = zoal::utils::function_scheduler<counter, 8, void *>;

matrix_type matrix;
scheduler_type scheduler;
uint8_t current_message = 0;

const uint64_t messages[][4] PROGMEM = {
        {0x3f66663e66663f00, 0x0000333333336e00, 0x00003e031e301f00, 0x00003333333e301f}, // Busy
        {0x7f46161e16060f00, 0x00003b6e66060f00, 0x00001e333f031e00, 0x00001e333f031e00}, // Free
        {0x1c36636363361c00, 0x00003b66663e060f, 0x00001e333f031e00, 0x00001f3333333300}, // Open
        {0x3333331e0c0c1e00, 0x00001e333f031e00, 0x00003e031e301f00, 0x183c3c1818001800}, // Yes!
        {0x63676f7b73636300, 0x00001e3333331e00, 0x183c3c1818001800, 0x0000000000000000}, // No!
        {0x1f36666666361f00, 0x00001e3333331e00, 0x00001f3333333300, 0x00001e333f031e00}, // Done
        {0x7f46161e16060f00, 0x00001e303e336e00, 0x0c000e0c0c0c1e00, 0x0e0c0c0c0c0c1e00}, // Fail
};
constexpr auto message_count = sizeof(messages) / sizeof(messages[0]);

void fill_matrix(const void *ptr) {
    auto src = reinterpret_cast<const uint8_t *>(ptr);
    uint8_t *dest = &matrix.data[0][0];
    for (uint8_t i = 0; i < sizeof(matrix.data); i++) {
        *dest++ = pgm_read_byte(src + i);
    }
}

void change_message(void *) {
    scheduler.schedule(1000, change_message);
    current_message++;
    if (current_message >= message_count) {
        current_message = 0;
    }

    fill_matrix(messages[current_message]);
    max7219::display(matrix);
}

typedef struct context {
    jsonlite_parser parser;
    jsonlite_builder builder;
} context;

#define MAX_JSON_DEPTH (16)

static void event_occurred(jsonlite_callback_context *ctx, jsonlite_event event) {
    auto *c = reinterpret_cast<context *>(ctx->client_state);
    switch (event) {
        case jsonlite_event_finished:
            break;
        case jsonlite_event_object_start:
            jsonlite_builder_object_begin(c->builder);
            break;
        case jsonlite_event_object_end:
            jsonlite_builder_object_end(c->builder);
            break;
        case jsonlite_event_array_start:
            jsonlite_builder_array_begin(c->builder);
            break;
        case jsonlite_event_array_end:
            jsonlite_builder_array_end(c->builder);
            break;
        default:
            break;
    }
}

static void token_callback(jsonlite_callback_context *ctx, jsonlite_token *t) {
    auto *c = reinterpret_cast<context *>(ctx->client_state);
    auto type = static_cast<jsonlite_token_type>(t->type & jsonlite_token_type_mask);
    switch (type) {
        case jsonlite_token_null:
            jsonlite_builder_null(c->builder);
            break;
        case jsonlite_token_true:
            jsonlite_builder_true(c->builder);
            break;
        case jsonlite_token_false:
            jsonlite_builder_false(c->builder);
            break;
        case jsonlite_token_key:
            jsonlite_builder_raw_key(c->builder, t->start, t->end - t->start);
            break;
        case jsonlite_token_number:
            jsonlite_builder_raw_value(c->builder, t->start, t->end - t->start);
            break;
        case jsonlite_token_string:
            jsonlite_builder_raw_string(c->builder, t->start, t->end - t->start);
            break;
        default:
            break;
    }
}

context ctx;
char json[] = R"({"a":null,"b":[1,2,3],"c":true,"d":{"a":1,"b":[]},"e":false,"f":["a","a","a"]})";
uint8_t parser_memory[jsonlite_parser_estimate_size(MAX_JSON_DEPTH)];
uint8_t builder_memory[jsonlite_builder_estimate_size(MAX_JSON_DEPTH)];

io_ring_buffer<uint8_t, usart> rb;

int uart_stream_write(jsonlite_stream stream, const void *data, size_t length) {
    auto ch = reinterpret_cast<const uint8_t *>(data);
    for (size_t i = 0; i < length; i++) {
        rb.put(*ch++);
    }

    return length;
}

void do_write(uint8_t v) {
    rb.put(v);
}

int main() {
    mcu::power<usart, timer, ir_timer, spi>::on();

    mcu::mux::usart<usart, mcu::pd_00, mcu::pd_01, mcu::pd_04>::on();
    mcu::cfg::usart<usart, 115200>::apply();

    mcu::cfg::timer<timer, zoal::periph::timer_mode::up, 64, 1, 0xFF>::apply();
    mcu::irq::timer<timer>::enable_overflow_interrupt();

//    mcu::cfg::spi<spi, 2>::apply();
//    mcu::mux::spi<spi, mcu::pb_03, mcu::pb_04, mcu::pb_05, mcu::pb_02>::on();
//    mcu::cfg::spi<spi>::apply();

    mcu::enable<usart, timer, ir_timer, spi>::on();

    zoal::utils::interrupts::on();

//    max7219::spi::enable();
//    max7219::init(matrix_type::devices);
//    max7219::send(matrix_type::devices, max7219::intensity0);

#if 0
    jsonlite_stream_struct out_stream;
    out_stream.write = &uart_stream_write;
    ctx.builder = jsonlite_builder_init(builder_memory, sizeof(builder_memory), &out_stream);
    jsonlite_builder_set_indentation(ctx.builder, 4);

    ctx.parser = jsonlite_parser_init(parser_memory, sizeof(parser_memory), jsonlite_null_buffer());
    jsonlite_parser_callbacks cbs;
    cbs.event_occurred = event_occurred;
    cbs.token_found = token_callback;
    cbs.context.client_state = &ctx;
    jsonlite_parser_set_callback(ctx.parser, &cbs);
    jsonlite_parser_tokenize(ctx.parser, json, sizeof(json));
#else
//    do_write('\r');
//    do_write('\n');
//
//    for (int i = 0; i < 3; i++) {
//        do_write('\r');
//        do_write('\n');
//        for (int j = 0; j < 10; j++) {
//            do_write('a' + j);
//            delay::ms(1);
//        }
//    }
//
//    do_write('\r');
//    do_write('\n');
#endif

    mcu::api::mode<zoal::gpio::pin_mode::output,
            zoal::pcb::ard_d05,
            zoal::pcb::ard_d06,
            zoal::pcb::ard_d07,
            zoal::pcb::ard_d08>();

    mcu::api::low<
            zoal::pcb::ard_d05,
            zoal::pcb::ard_d06,
            zoal::pcb::ard_d07,
            zoal::pcb::ard_d08>();

    rb.put('\r');
    rb.put('\n');
    for (int j = 0; j < 50; j++) {
        for (int i = 0; i < 50; i++) {
            rb.put('A' + i);
        }
        rb.put('\r');
        rb.put('\n');
    }
    rb.put('\r');
    rb.put('\n');
    rb.put('-');

//    usart_01_tx_buffer::write_byte('T');

    while (true) {
        delay::ms(500);
        zoal::pcb::ard_d05::toggle();
    }

    return 0;
}

ISR(TIMER0_OVF_vect) {
    irq_handler::increment();
}

ISR(USART_RX_vect) {
    usart::rx_handler<usart_01_rx_buffer>();
}

ISR(USART_UDRE_vect) {
    uint8_t value = '+';
    if (rb.get(value)) {
        UDR0 = value;
    } else {
        UCSR0B &= ~(1 << UDRIE0);
    }
}
