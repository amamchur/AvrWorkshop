#include "printer/adpt1.hpp"
#include "printer/xlp504.hpp"

#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <zoal/board/arduino_leonardo.hpp>
#include <zoal/ic/hd44780.hpp>
#include <zoal/io/analog_keypad.hpp>
#include <zoal/periph/rx_null_buffer.hpp>
#include <zoal/shield/uno_lcd.hpp>
#include <zoal/utils/logger.hpp>
#include <zoal/utils/ms_counter.hpp>
#include <zoal/utils/tool_set.hpp>

template<class First, class... Rest>
struct max_type_size {
    using other = max_type_size<Rest...>;
    static constexpr auto value = sizeof(First) > other::value ? sizeof(First) : other::value;
};

template<class First>
struct max_type_size<First> {
    static constexpr auto value = sizeof(First);
};

enum class lcd_display_mode {
    printer_info,
    rx_tx_info,
};

volatile uint32_t timer0_millis = 0;
volatile uint16_t adcValue = 0xFFFF;
volatile bool running = true;

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
using lcd = shield::lcd;
using keypad = shield::keypad;

constexpr auto lcd_rows = lcd::address_selector::rows;
constexpr auto lcd_columns = lcd::address_selector::columns;
char lcd_mem[lcd_rows][lcd_columns] = {{' '}};

tools::function_scheduler<8> timeout;

int current_printer_id = 0;
lcd_display_mode display_mode = lcd_display_mode::printer_info;

constexpr auto printer_buffer_size = max_type_size<printer::adpt1, printer::xlp504>::value;
uint8_t printer_buffer[printer_buffer_size];
printer::base *current_printer = nullptr;

uint32_t serial_number = 0;
wchar_t serial_number_str[] = L"0000000000";

constexpr auto
        usb_buffer_size =
        max_type_size<USB_Descriptor_Header_t, USB_Descriptor_Device_t, printer::adpt1::usb_configuration, printer::xlp504::usb_configuration>::
        value +
        32;
uint8_t usb_buffer[usb_buffer_size];

uint16_t eeprom_buttons[shield::button_count] __attribute__((section(".eeprom"))) = {637, 411, 258, 101, 0};
wchar_t eeprom_sn[sizeof(serial_number_str)] __attribute__((section(".eeprom")));
int eeprom_printer_id[1] __attribute__((section(".eeprom")));

void update_display();

void lcd_mem_print(int r, int c, const char *str) {
    for (auto i = 0; i + c < lcd_columns && str[i] != 0; i++) {
        lcd_mem[r][i + c] = str[i];
    }
}

void lcd_mem_print(int r, int c, const wchar_t *str) {
    for (auto i = 0; i + c < lcd_columns && str[i] != 0; i++) {
        lcd_mem[r][i + c] = static_cast<char>(str[i]);
    }
}

void transfer_lcd_mem() {
    for (auto i = 0; i < lcd_rows; i++) {
        shield::lcd::move(i, 0);
        for (auto j = 0; j < lcd_columns; j++) {
            shield::lcd::write_byte(lcd_mem[i][j]);
        }
    }
}

void read_eeprom_data() {
    eeprom_read_block(keypad::values, eeprom_buttons, sizeof(keypad::values));
    eeprom_read_block(serial_number_str, eeprom_sn, sizeof(serial_number_str));
    eeprom_read_block(&current_printer_id, eeprom_printer_id, sizeof(current_printer_id));

    constexpr int l = sizeof(serial_number_str) / sizeof(serial_number_str[0]) - 1;
    serial_number_str[l] = 0x00;
    serial_number = 0;
    for (auto i = 0; i < l; i++) {
        if (serial_number_str[i] < '0' || serial_number_str[i] > '9') {
            serial_number_str[i] = '0';
        }

        serial_number = serial_number * 10 + (serial_number_str[i] - '0');
    }

    if (current_printer_id > 1) {
        current_printer_id = 1;
    }

    if (current_printer_id < 0) {
        current_printer_id = 0;
    }
}

void write_eeprom_data() {
    eeprom_write_block(keypad::values, eeprom_buttons, sizeof(keypad::values));
    eeprom_write_block(serial_number_str, eeprom_sn, sizeof(serial_number_str));
    eeprom_write_block(&current_printer_id, eeprom_printer_id, sizeof(current_printer_id));
}

void initialize_hardware() {
    clock_prescale_set(clock_div_1);

    mcu::power<usart, timer>::on();

    mcu::mux::usart<usart, zoal::pcb::ard_d00, zoal::pcb::ard_d01, mcu::pd_05>::on();
    mcu::cfg::usart<usart, 115200>::apply();

    mcu::cfg::timer<timer, zoal::periph::timer_mode::up, 64, 1, 0xFF>::apply();
    mcu::irq::timer<timer>::enable_overflow_interrupt();

    mcu::cfg::adc<adc>::apply();

    mcu::enable<usart, timer, adc>::on();

    zoal::utils::interrupts::on();
}

void button_handler(size_t button, zoal::io::button_event event) {
    if (event != zoal::io::button_event::press) {
        return;
    }

    switch (button) {
        case shield::select_btn:
            break;
        case shield::left_btn:
            break;
        case shield::down_btn:
            display_mode = (lcd_display_mode) ((int) display_mode + 1);
            break;
        case shield::up_btn:
            display_mode = (lcd_display_mode) ((int) display_mode - 1);
            break;
        case shield::right_btn:
            break;
        default:
            break;
    }

    if (display_mode < lcd_display_mode::printer_info) {
        display_mode = lcd_display_mode::printer_info;
    }

    if (display_mode > lcd_display_mode::rx_tx_info) {
        display_mode = lcd_display_mode::rx_tx_info;
    }

    timeout.schedule(0, update_display);
}

void inc_current_printer(int d) {
    current_printer_id += d;
    if (current_printer_id > 1) {
        current_printer_id = 1;
    }

    if (current_printer_id < 0) {
        current_printer_id = 0;
    }

    memset(lcd_mem, ' ', sizeof(lcd_mem));
    lcd_mem_print(0, 0, "Printer");
    if (current_printer_id == 0) {
        lcd_mem_print(1, 0, "> ADTP1");
    } else {
        lcd_mem_print(1, 0, "> XLP504");
    }
    transfer_lcd_mem();
}

void inc_serial_number(int d) {
    serial_number += d;

    constexpr int count = sizeof(serial_number_str) / sizeof(serial_number_str[0]);
    auto p = &serial_number_str[count - 1];
    *p-- = 0;

    auto v = serial_number;
    while (p >= serial_number_str) {
        *p = static_cast<wchar_t>('0' + v % 10);
        v /= 10;
        p--;
    }

    memset(lcd_mem, ' ', sizeof(lcd_mem));
    lcd_mem_print(0, 0, "Enter SN");
    lcd_mem_print(1, 0, serial_number_str);
    transfer_lcd_mem();
}

void configuration_menu() {
    mcu::mux::adc<adc, shield::analog_pin>::on();

    uint16_t value = adc::read();
    if (value > 10) {
        return;
    }

    do {
        value = adc::read();
    } while (value > 900);

    memset(lcd_mem, ' ', sizeof(lcd_mem));
    lcd_mem_print(0, 0, "Entering menu...");
    lcd_mem_print(1, 0, "Release keys pls");
    transfer_lcd_mem();
    tools::delay::ms(3000);

    shield::calibrate(true);
    inc_serial_number(0);

    bool done = false;
    do {
        value = adc::read();
        shield::handle_keypad(
                [&done](size_t button, zoal::io::button_event event) {
                    if (event != zoal::io::button_event::press) {
                        return;
                    }

                    switch (button) {
                        case shield::select_btn:
                            done = true;
                            break;
                        case shield::down_btn:
                            inc_serial_number(-1);
                            break;
                        case shield::up_btn:
                            inc_serial_number(1);
                            break;
                        default:
                            break;
                    }
                },
                value);
    } while (!done);

    done = false;
    inc_current_printer(0);
    do {
        value = adc::read();
        shield::handle_keypad(
                [&done](size_t button, zoal::io::button_event event) {
                    if (event != zoal::io::button_event::press) {
                        return;
                    }

                    switch (button) {
                        case shield::select_btn:
                            done = true;
                            break;
                        case shield::down_btn:
                            inc_current_printer(1);
                            break;
                        case shield::up_btn:
                            inc_current_printer(-1);
                            break;
                        default:
                            break;
                    }
                },
                value);
    } while (!done);

    write_eeprom_data();

    shield::lcd::clear();
    shield::lcd::write("Rebooting...");

//    wdt_enable(WDTO_1S);
//    while (1);
}

void run() {
    using namespace zoal::io;
    using namespace zoal::gpio;

    if (adcValue != 0xFFFF) {
        zoal::utils::interrupts interrupts(false);
        shield::handle_keypad(button_handler, adcValue);
        adcValue = 0xFFFF;
        shield::adc::start();
    }

    uint8_t received = PRNT_Device_BytesReceived(current_printer->interface());
    if (received > 0) {
        timeout.schedule(0, update_display);
    }
    for (; received > 0; received--) {
        int16_t byte = PRNT_Device_ReceiveByte(current_printer->interface());
        current_printer->process_byte(static_cast<uint8_t>(byte));
    }

    PRNT_Device_USBTask(current_printer->interface());
    USB_USBTask();
}

void update_display() {
    memset(lcd_mem, ' ', sizeof(lcd_mem));

    switch (display_mode) {
        case lcd_display_mode::printer_info:
            lcd_mem_print(0, 0, current_printer->name());
            lcd_mem_print(1, 0, "SN: ");
            lcd_mem_print(1, 4, serial_number_str);
            break;
        case lcd_display_mode::rx_tx_info: {
            lcd_mem_print(0, 0, "RX:");
            lcd_mem[0][lcd_columns - 1] = '0';
            auto v = current_printer->rx_bytes();
            auto pos = lcd_columns;
            lcd_mem[0][--pos] = '0';
            while (v != 0) {
                lcd_mem[0][pos--] = static_cast<char>('0' + v % 10);
                v /= 10;
            }
            lcd_mem_print(1, 0, "TX:");
            v = current_printer->tx_bytes();
            pos = lcd_columns;
            lcd_mem[1][--pos] = '0';
            while (v != 0) {
                lcd_mem[1][pos--] = static_cast<char>('0' + v % 10);
                v /= 10;
            }
            break;
        }
        default:
            break;
    }
    transfer_lcd_mem();
}

int main() {
    using namespace zoal::io;
    using namespace zoal::gpio;

    MCUSR = 0;
    read_eeprom_data();

    initialize_hardware();
    shield::gpio_cfg();
    shield::init();

    configuration_menu();

    printer::base::shared_memory = printer_buffer;
    switch (current_printer_id) {
        case 0:
            current_printer = new printer::adpt1();
            break;
        default:
            current_printer = new printer::xlp504();
            break;
    }
    current_printer->init();

    shield::adc::enable_interrupt();
    shield::adc::start();

    timeout.schedule(0, update_display);

    USB_Init();
    while (running) {
        run();
        timeout.handle();
    }
    USB_Disable();
}

ISR(WDT_vect) {
}

ISR(ADC_vect) {
    adcValue = shield::adc::value();
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

extern "C" uint16_t
CALLBACK_USB_GetDescriptor(const uint16_t wValue, const uint16_t wIndex, const void **const DescriptorAddress) {
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
                            sizeof(serial_number_str) - sizeof(serial_number_str[0]));
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
