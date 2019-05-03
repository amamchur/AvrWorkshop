#pragma once

#ifndef MATRIX_HELPERS_HPP
#define MATRIX_HELPERS_HPP

#include <stdint.h>
#include <avr/pgmspace.h>

const PROGMEM uint8_t DotsData[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00,
  0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

class dots {
  protected:
    uint8_t value(uint8_t device, uint8_t row) const {
      return pgm_read_byte_near(DotsData + device * 8 + row);
    }
};

class place_dots : private dots {
  public:
    uint8_t operator ()(uint8_t device, uint8_t row) const {
      return value(device, row);
    }
};

class remove_dots : private dots {
  public:
    uint8_t operator ()(uint8_t , uint8_t row) const {
      return ~value(1, row);
    }
};

#endif

