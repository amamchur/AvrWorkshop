#ifndef AVR_WORKSHOP_FONTS_HPP
#define AVR_WORKSHOP_FONTS_HPP

#include <stdint.h>
#include <stddef.h>

#ifdef __AVR_ARCH__
#include <avr/pgmspace.h>
#else
#define PROGMEM
#endif

extern const char start_glyph_code;
extern const uint64_t font_glyphs[] PROGMEM;
extern const size_t font_glyph_count;

#endif
