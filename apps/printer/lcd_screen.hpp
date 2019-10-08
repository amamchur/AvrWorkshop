#ifndef AVR_WORKSHOP_LCD_SCREEN_HPP
#define AVR_WORKSHOP_LCD_SCREEN_HPP

#include <avr/pgmspace.h>
#include <string.h>

template<class LCD>
class lcd_screen {
public:
    static constexpr auto rows = LCD::address_selector::rows;
    static constexpr auto columns = LCD::address_selector::columns;

    static void clear() {
        memset(mem, ' ', sizeof(mem));
    }

    static void copy_pgm(int r, const void *pm) {
        auto p = reinterpret_cast<const char *>(pm);
        auto ch = pgm_read_byte(p++);
        auto dst = &mem[r][0];
        for (; ch != 0; p++) {
            *dst++ = ch;
            ch = pgm_read_byte(p);
        }
    }

    static void copy_pgm(const void *pm) {
        auto p = reinterpret_cast<const char *>(pm);
        auto ch = pgm_read_byte(p++);
        auto dst = &mem[0][0];
        for (; ch != 0; p++) {
            *dst++ = ch;
            ch = pgm_read_byte(p);
        }
    }

    static void print_right(int r, size_t value) {
        auto pos = columns;
        mem[r][--pos] = '0';
        while (value != 0) {
            mem[r][pos--] = static_cast<char>('0' + value % 10);
            value /= 10;
        }
    }

    template<class T>
    static void print(int r, int c, const T *str) {
        for (size_t i = 0; str[i] != 0; i++) {
            mem[r][i + c] = static_cast<T>(str[i]);
        }
    }

    static void flush() {
        for (size_t i = 0; i < rows; i++) {
            LCD::move(i, 0);
            for (size_t j = 0; j < columns; j++) {
                LCD::write_byte(mem[i][j]);
            }
        }
    }

    static char mem[rows][columns];
};

template<class LCD>
char lcd_screen<LCD>::mem[rows][columns];

#endif
