#include "animator.hpp"
#include "parser.hpp"
#include "executer.hpp"
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

static constexpr size_t DeviceCount = 8;
static constexpr int MsgLength = 128;
volatile uint32_t milliseconds = 0;

using pcb = zoal::pcb;
using mcu = pcb::mcu;
using counter = zoal::utils::ms_counter<decltype(milliseconds), &milliseconds>;
using timer = mcu::timer_00;
using ir_timer = mcu::timer_02;
using adc = mcu::adc_00;
using spi = mcu::spi_00;
using irq_handler = counter::handler<mcu::frequency, 64, timer>;
using usart = mcu::usart_00;
using tx_buffer = usart::default_tx_buffer<16>;
using rx_buffer = usart::default_rx_buffer<16>;
//using rx_buffer = usart::null_rx_buffer;

using spi = mcu::spi_00;
using sspi = zoal::periph::tx_software_spi<mcu::pb_03, mcu::pb_02>;

using logger = zoal::utils::plain_logger<tx_buffer, zoal::utils::log_level::trace>;
using tools = zoal::utils::tool_set<mcu, counter, logger>;
using delay = tools::delay;
using matrix_type = zoal::ic::max72xx_data<DeviceCount>;
using matrix_type_ext = zoal::ic::max72xx_data<DeviceCount + 1>;
using max7219 = zoal::ic::max72xx<spi, zoal::pcb::ard_d10>;
using scheduler_type = zoal::utils::function_scheduler<counter, 8, void *>;

using parser_type = parser<MsgLength>;
using animator_type = animator<MsgLength, DeviceCount, max7219, scheduler_type>;
using executer_type = executer<MsgLength, DeviceCount, max7219, animator_type, logger>;

scheduler_type scheduler;
parser_type clp;
animator_type anim(scheduler);
executer_type exect(anim);

void callback(base_parser *p, parse_event evnt) {
    exect.handle(p, evnt);
}

int main() {
    mcu::power<usart, timer, spi>::on();

    // Rx - pcb::ard_d00
    // Rx - pcb::ard_d01
    mcu::mux::usart<usart, pcb::ard_d00, pcb::ard_d01>::on();
    mcu::cfg::usart<usart, 115200>::apply();

    mcu::cfg::timer<timer, zoal::periph::timer_mode::up, 64, 1, 0xFF>::apply();
    mcu::irq::timer<timer>::enable_overflow_interrupt();

    // MISO - pcb::ard_d11
    // MOSI - pcb::ard_d12
    // SCLK - pcb::ard_d13
    // CS   - pcb::ard_d10
    mcu::mux::spi<spi, pcb::ard_d11, pcb::ard_d12, pcb::ard_d13, pcb::ard_d10>::on();
    mcu::cfg::spi<spi, 2>::apply();
    mcu::cfg::spi<spi>::apply();

    mcu::enable<usart, timer, spi>::on();

    mcu::irq::on();

    clp.callback(&callback);

    max7219::spi::enable();
    max7219::init(matrix_type::devices);
    max7219::send(matrix_type::devices, max7219::intensity0);

    anim.message("Hello world!!!    ");
    anim.start();

    while (true) {
        uint8_t rx_byte = 0;
        auto result = rx_buffer::pop_front(rx_byte);
        if (result) {
            clp.push(rx_byte);
        }

        scheduler.handle();
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
