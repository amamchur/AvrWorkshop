#ifndef AVR_WORKSHOP_LOCALIZATION_HPP
#define AVR_WORKSHOP_LOCALIZATION_HPP

#include <avr/pgmspace.h>
#include <stdint.h>

extern const char PROGMEM menu_greeting[];
extern const char PROGMEM text_choose_model[];
extern const char PROGMEM text_choose_serial[];
extern const char PROGMEM text_change_printer[];
extern const char PROGMEM text_change_serial[];
extern const char PROGMEM text_calibrate[];
extern const char PROGMEM text_press_select[];
extern const char PROGMEM text_press_left[];
extern const char PROGMEM text_press_down[];
extern const char PROGMEM text_press_up[];
extern const char PROGMEM text_press_right[];
extern const char PROGMEM text_complete[];
extern const char PROGMEM text_sn_format[];
extern const char PROGMEM text_adtp1_name[];
extern const char PROGMEM text_xlp504_name[];

extern const uint8_t PROGMEM up_down_char[];

template <class LCD>
void create_lcd_char(uint8_t pos, const void *pgmem) {
    constexpr uint8_t buffer_size = 8;
    uint8_t buffer[buffer_size];
    auto p = reinterpret_cast<const char *>(pgmem);
    auto dst = buffer;
    for (uint8_t i = 0; i < buffer_size; i++) {
        *dst++ = pgm_read_byte(p + i);
    }

    LCD::create_char(pos, buffer);
}

template <class LCD>
void create_custom_lcd_char() {
    create_lcd_char<LCD>(1, up_down_char);
}

#endif

