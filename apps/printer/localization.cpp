#include "localization.hpp"

const char PROGMEM menu_greeting[] = "Entering menu..."
                                     "Release keys pls";
const char PROGMEM text_choose_model[] = "Choose Model   \x01";
const char PROGMEM text_choose_serial[] = "Choose Serial  \x01";
const char PROGMEM text_change_printer[] = "Change Printer  ";
const char PROGMEM text_change_serial[] = "Change Serial No";
const char PROGMEM text_press_select[] = "Press Select    ";
const char PROGMEM text_calibrate[] = "Calibration     ";
const char PROGMEM text_press_left[] = "Press Left      ";
const char PROGMEM text_press_down[] = "Press Down      ";
const char PROGMEM text_press_up[] = "Press Up        ";
const char PROGMEM text_press_right[] = "Press Right     ";
const char PROGMEM text_complete[] = "Complete!       ";
const char PROGMEM text_sn_format[] = "SN    0000000000";
const char PROGMEM text_adtp1_name[] = "ADTP1  12EE:1043";
const char PROGMEM text_xlp504_name[] = "XLP503 2BFE:0E45";

const uint8_t PROGMEM up_down_char[] = {0x04, 0x0E, 0x1F, 0x00, 0x00, 0x1F, 0x0E, 0x04};
