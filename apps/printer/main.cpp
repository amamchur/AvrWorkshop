#include <avr/power.h>

#include "pcb_cfg.h"
#include "menu.h"

void initialize_hardware() {
    clock_prescale_set(clock_div_1);

    mcu::power<usart, timer>::on();

    mcu::mux::usart<usart, zoal::pcb::ard_d00, zoal::pcb::ard_d01, mcu::pd_05>::on();
    mcu::cfg::usart<usart, 115200>::apply();

    mcu::cfg::timer<timer, zoal::periph::timer_mode::up, 64, 1, 0xFF>::apply();
    mcu::irq::timer<timer>::enable_overflow_interrupt();

    mcu::mux::adc<adc, shield::analog_pin>::on();
    mcu::cfg::adc<adc>::apply();

    mcu::enable<usart, timer, adc>::on();

    zoal::utils::interrupts::on();
}

int main() {
    using namespace zoal::io;
    using namespace zoal::gpio;

    read_eeprom_data();

    initialize_hardware();
    shield::gpio_cfg();
    shield::init();
    shield::adc::enable_interrupt();

    uint16_t value = adc::read();
    if (value < 10) {
        start_hartware_configuration();
    } else {
        init_printer();
        start_main_menu();
    }

    while (true) {
        scheduler.handle();
    }
}

ISR(ADC_vect) {
    adc_value = shield::adc::value();
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

