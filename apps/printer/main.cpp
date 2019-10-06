#include <avr/power.h>

#include "printer/adpt1.hpp"
#include "printer/xlp504.hpp"
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

void handle_usb() {
    timeout.schedule(0, handle_usb);

    uint8_t received = PRNT_Device_BytesReceived(current_printer->interface());
//    if (received > 0) {
//        timeout.schedule(0, update_display__);
//    }

    for (; received > 0; received--) {
        int16_t byte = PRNT_Device_ReceiveByte(current_printer->interface());
        current_printer->process_byte(static_cast<uint8_t>(byte));
    }

    PRNT_Device_USBTask(current_printer->interface());
    USB_USBTask();
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

    while (running) {
        timeout.handle();
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

extern "C" void EVENT_USB_Device_Connect(void) {}

extern "C" void EVENT_USB_Device_Disconnect() {}

extern "C" void EVENT_USB_Device_ConfigurationChanged() {
    PRNT_Device_ConfigureEndpoints(current_printer->interface());
}

extern "C" void EVENT_USB_Device_ControlRequest() {
    PRNT_Device_ProcessControlRequest(current_printer->interface());
}

#if defined(USE_RAM_DESCRIPTORS)

constexpr auto usb_buffer_size = zoal::ct::max_type_size<
        USB_Descriptor_Header_t,
        USB_Descriptor_Device_t,
        printer::adpt1::usb_configuration,
        printer::xlp504::usb_configuration
>::value + 32;
uint8_t usb_buffer[usb_buffer_size];

extern "C" uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue, const uint16_t wIndex, const void **const DescriptorAddress) {
    static_assert(sizeof(usb_buffer) >= sizeof(USB_Descriptor_Device_t), "");
    static_assert(sizeof(usb_buffer) >= sizeof(printer::adpt1::usb_configuration), "");

    const uint8_t DescriptorType = (wValue >> 8u);
    const uint8_t DescriptorNumber = (wValue & 0xFFu);

    const void *address = nullptr;
    uint16_t size = NO_DESCRIPTOR;

    switch (DescriptorType) {
        case DTYPE_Device:
            size = current_printer->get_descriptor(usb_buffer, sizeof(usb_buffer));
            address = usb_buffer;
            break;
        case DTYPE_Configuration:
            size = current_printer->get_configuration(usb_buffer, sizeof(usb_buffer));
            address = usb_buffer;
            break;
        case DTYPE_String:
            switch (DescriptorNumber) {
                case printer::base::string_id_language:
                    size = current_printer->get_language_string(usb_buffer, sizeof(usb_buffer));
                    address = usb_buffer;
                    break;
                case printer::base::string_id_manufacturer:
                    size = current_printer->get_manufacturer_string(usb_buffer, sizeof(usb_buffer));
                    address = usb_buffer;
                    break;
                case printer::base::string_id_product:
                    size = current_printer->get_product_string(usb_buffer, sizeof(usb_buffer));
                    address = usb_buffer;
                    break;
                case printer::base::string_id_serial_number:
                    size = printer::base::fill_descriptor_string(
                            usb_buffer, sizeof(usb_buffer), serial_number_str,
                            serial_number_size - sizeof(serial_number_str[0]));
                    address = usb_buffer;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    *DescriptorAddress = address;
    return size;
}

#endif
