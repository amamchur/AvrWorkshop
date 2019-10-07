#include "pcb_cfg.h"
#include <avr/eeprom.h>

volatile uint32_t timer0_millis = 0;
volatile uint16_t adc_value = 0xFFFF;

printer_device current_printer_id;
uint32_t serial_number = 0;
wchar_t serial_number_str[] = L"0000000000";
size_t serial_number_size = sizeof(serial_number_str);
uint8_t printer_buffer[printer_buffer_size];
printer::base *current_printer = nullptr;

tools::function_scheduler<8> scheduler;

uint8_t eeprom_buttons[sizeof(keypad::values)] __attribute__((section(".eeprom")));
uint8_t eeprom_printer_id[sizeof(current_printer_id)] __attribute__((section(".eeprom")));
uint8_t eeprom_sn[sizeof(serial_number)] __attribute__((section(".eeprom")));

void read_eeprom_data() {
    eeprom_read_block(keypad::values, eeprom_buttons, sizeof(keypad::values));
    eeprom_read_block(&serial_number, eeprom_sn, sizeof(serial_number));
    eeprom_read_block(&current_printer_id, eeprom_printer_id, sizeof(current_printer_id));

    current_printer_id = shift_enum(current_printer_id, 0, printer_device::count);
    format_serial_number();
}

void write_eeprom_data() {
    eeprom_write_block(keypad::values, eeprom_buttons, sizeof(keypad::values));
    eeprom_write_block(&serial_number, eeprom_sn, sizeof(serial_number));
    eeprom_write_block(&current_printer_id, eeprom_printer_id, sizeof(current_printer_id));
}

void format_serial_number() {
    auto ptr = serial_number_str + sizeof(serial_number_str) / sizeof(serial_number_str[0]) - 1;
    auto v = serial_number;
    for (*ptr-- = 0x00; ptr >= serial_number_str; ptr--) {
        *ptr = static_cast<wchar_t >('0' + v % 10);
        v /= 10;
    }
}

void inc_serial_number(int d) {
    serial_number += d;

    format_serial_number();
}

void init_printer() {
    printer::base::shared_memory = printer_buffer;
    delete current_printer;
    switch (current_printer_id) {
        case printer_device::adtp1:
            current_printer = new printer::adpt1();
            break;
        default:
            current_printer = new printer::xlp504();
            break;
    }

    current_printer->init();
    USB_Init();

    scheduler.schedule(0, handle_usb);
}

void deinit_printer() {
    USB_Disable();
}

constexpr auto usb_buffer_size = zoal::ct::max_type_size<
        USB_Descriptor_Header_t,
        USB_Descriptor_Device_t,
        printer::adpt1::usb_configuration,
        printer::xlp504::usb_configuration
>::value + 32;
uint8_t usb_buffer[usb_buffer_size];

void handle_usb() {
    scheduler.schedule(0, handle_usb);

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

extern "C" void EVENT_USB_Device_ConfigurationChanged() {
    PRNT_Device_ConfigureEndpoints(current_printer->interface());
}

extern "C" void EVENT_USB_Device_ControlRequest() {
    PRNT_Device_ProcessControlRequest(current_printer->interface());
}

extern "C" uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue, const uint16_t wIndex, const void **const DescriptorAddress) {
    uint8_t DescriptorType = (wValue >> 8u);
    uint8_t DescriptorNumber = (wValue & 0xFFu);

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
