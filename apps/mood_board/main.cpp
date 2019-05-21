#include "../../libs/jsonlite/jsonlite.h"
#include "fonts.hpp"

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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

volatile uint32_t milliseconds = 0;

using mcu = zoal::pcb::mcu;
using counter = zoal::utils::ms_counter<decltype(milliseconds), &milliseconds>;
using timer = mcu::timer_00;
using ir_timer = mcu::timer_02;
using adc = mcu::adc_00;
using spi = mcu::spi_00;
using irq_handler = counter::handler<mcu::frequency, 64, timer>;
using usart = mcu::usart_00;
using tx_buffer = usart::default_tx_buffer<16>;
//using rx_buffer = usart::default_rx_buffer<16>;
using rx_buffer = usart::null_rx_buffer;

using spi = mcu::spi_00;
using sspi = zoal::periph::tx_software_spi<mcu::pb_03, mcu::pb_02>;

using logger = zoal::utils::plain_logger<tx_buffer, zoal::utils::log_level::trace>;
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
    //    scheduler.schedule(1000, change_message);
    current_message++;
    if (current_message >= message_count) {
        current_message = 0;
    }

    fill_matrix(messages[current_message]);
    max7219::display(matrix);
}

typedef struct context {
} context;

#define MAX_CHUNK_SIZE 64
#define MAX_JSON_DEPTH 6
#define MAX_JSON_TOKEN_SIZE 20

context ctx;
uint8_t parser_memory[jsonlite_parser_estimate_size(MAX_JSON_DEPTH)];
uint8_t buffer_memory[jsonlite_buffer_static_size_ext(MAX_JSON_TOKEN_SIZE, MAX_CHUNK_SIZE)];
jsonlite_buffer buffer;
jsonlite_parser parser;

static void process_number(jsonlite_token *pToken);

static void process_string(jsonlite_token *pToken);

static void event_occurred(jsonlite_callback_context *ctx, jsonlite_event event) {
    //    auto *c = reinterpret_cast<context *>(ctx->client_state);
    switch (event) {
    case jsonlite_event_finished: {
        auto r = jsonlite_parser_get_result(ctx->parser);
        if (r == jsonlite_result_ok) {
            logger::info() << "Success!!!";
        }
        break;
    }
    case jsonlite_event_object_start:
        break;
    case jsonlite_event_object_end:
        break;
    case jsonlite_event_array_start:
        break;
    case jsonlite_event_array_end:
        break;
    default:
        break;
    }
}

static void token_callback(jsonlite_callback_context *ctx, jsonlite_token *t) {
    //    auto *c = reinterpret_cast<context *>(ctx->client_state);
    auto type = static_cast<jsonlite_token_type>(t->type & jsonlite_token_type_mask);
    switch (type) {
    case jsonlite_token_null:
        break;
    case jsonlite_token_true:
        break;
    case jsonlite_token_false:
        break;
    case jsonlite_token_key:
        break;
    case jsonlite_token_number:
        process_number(t);
        break;
    case jsonlite_token_string:
        process_string(t);
        break;
    default:
        break;
    }
}

static void process_string(jsonlite_token *pToken) {
    logger::info() << "String!!!";

    auto ch = pToken->start;
    for (int i = 0; i < 4 && ch != pToken->end; i++, ch++) {
        bool valid = *ch >= 'A' && *ch <= 'Z';
        valid = valid || (*ch >= 'a' && *ch <= 'z');

        if (!valid) {
            continue;
        }

        auto src = reinterpret_cast<const uint8_t *>(font_letter + (*ch - 'A'));
        auto *dest = matrix.data[i];
        for (uint8_t j = 0; j < 8; j++) {
            *dest++ = pgm_read_byte(src + j);
        }
    }

    max7219::display(matrix);
}

int text_offset = 0;

void display_test(void *) {
    text_offset++;

    if (text_offset > 50) {
        text_offset = 0;
    }

    auto ch = "ABCD";
    for (int i = 0; i < 4 && *ch; i++, ch++) {
        bool valid = *ch >= 'A' && *ch <= 'Z';
        valid = valid || (*ch >= 'a' && *ch <= 'z');

        if (!valid) {
            continue;
        }

        auto src = reinterpret_cast<const uint8_t *>(font_letter + (*ch - 'A') + text_offset);
        auto *dest = matrix.data[i];
        for (uint8_t j = 0; j < 8; j++) {
            *dest++ = pgm_read_byte(src + j);
        }
    }

    max7219::display(matrix);

    scheduler.schedule(500, display_test);
}

static void process_number(jsonlite_token *pToken) {
    auto value = jsonlite_token_to_long(pToken);
    if (value < message_count) {
        current_message = value;
    }

    logger::info() << "Number: " << value << " cm:" << current_message;

    fill_matrix(messages[current_message]);
    max7219::display(matrix);
}

void init_parser() {
    jsonlite_parser_callbacks cbs;
    cbs.event_occurred = event_occurred;
    cbs.token_found = token_callback;
    cbs.context.client_state = &ctx;

    buffer = jsonlite_buffer_static_init(buffer_memory, sizeof(buffer_memory));
    parser = jsonlite_parser_init(parser_memory, sizeof(parser_memory), buffer);
    jsonlite_parser_set_callback(parser, &cbs);
}

int main() {
    mcu::power<usart, timer, spi>::on();

    mcu::mux::usart<usart, mcu::pd_00, mcu::pd_01, mcu::pd_04>::on();
    mcu::cfg::usart<usart, 115200>::apply();

    mcu::cfg::timer<timer, zoal::periph::timer_mode::up, 64, 1, 0xFF>::apply();
    mcu::irq::timer<timer>::enable_overflow_interrupt();

    mcu::cfg::spi<spi, 2>::apply();
    mcu::mux::spi<spi, mcu::pb_03, mcu::pb_04, mcu::pb_05, mcu::pb_02>::on();
    mcu::cfg::spi<spi>::apply();

    mcu::enable<usart, timer, spi>::on();

    mcu::irq::on();

    max7219::spi::enable();
    max7219::init(matrix_type::devices);
    max7219::send(matrix_type::devices, max7219::intensity0);

    logger::info() << "Ready!";

    //    jsonlite_result result = jsonlite_result_ok;

    scheduler.schedule(0, display_test);
    while (true) {
        scheduler.handle();

        //        switch (result) {
        //            case jsonlite_result_unknown:
        //            case jsonlite_result_end_of_stream:
        //                break;
        //            default:
        //                result = jsonlite_result_unknown;
        //                init_parser();
        //                break;
        //        }
        //
        //        rx_buffer::value_type value;
        //        if (rx_buffer::pop_front(value)) {
        //            result = jsonlite_parser_tokenize(parser, &value, sizeof(value));
        //        }
    }

    return 0;
}

ISR(TIMER0_OVF_vect) {
    irq_handler::increment();
}

ISR(USART_RX_vect) {
    usart::rx_handler_v2<rx_buffer>();
}

ISR(USART_UDRE_vect) {
    usart::tx_handler_v2<tx_buffer>();
}

#pragma clang diagnostic pop
