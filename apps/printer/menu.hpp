#ifndef AVR_WORKSHOP_MENU_HPP
#define AVR_WORKSHOP_MENU_HPP

enum class menu_option {
    printer_info,
    rx_tx_info,
    change_printer,
    change_serial,

    count
};

void start_hartware_configuration();

void start_main_menu();

void start_printer_select();

void update_rx_tx_display();

#endif
