#include "fonts.hpp"

const uint64_t font_letter[] PROGMEM = {
        0xc1e33333f333300,
        0x3f66663e66663f00,
        0x3c66030303663c00,
        0x1f36666666361f00,
        0x7f46161e16467f00,
        0x7f46161e16060f00,
        0x3c66030373667c00,
        0x3333333f33333300,
        0x1e0c0c0c0c0c1e00,
        0x7830303033331e00,
        0x6766361e36666700,
        0xf06060646667f00,
        0x63777f7f6b636300,
        0x63676f7b73636300,
        0x1c36636363361c00,
        0x3f66663e06060f00,
        0x1e3333333b1e3800,
        0x3f66663e36666700,
        0x1e33070e38331e00,
        0x3f2d0c0c0c0c1e00,
        0x3333333333333f00,
        0x33333333331e0c00,
        0x6363636b7f776300,
        0x6363361c1c366300,
        0x3333331e0c0c1e00,
        0x7f6331184c667f00,
        0x0, //[
        0x0, // "\"
        0x0, //]
        0x0, //^
        0x0, //_
        0x0, //`
        0x1e303e336e00,
        0x706063e66663b00,
        0x1e3303331e00,
        0x3830303e33336e00,
        0x1e333f031e00,
        0x1c36060f06060f00,
        0x6e33333e301f,
        0x706366e66666700,
        0xc000e0c0c0c1e00,
        0x300030303033331e,
        0x70666361e366700,
        0xe0c0c0c0c0c1e00,
        0x337f7f6b6300,
        0x1f3333333300,
        0x1e3333331e00,
        0x3b66663e060f,
        0x6e33333e3078,
        0x3b6e66060f00,
        0x3e031e301f00,
        0x80c3e0c0c2c1800,
        0x333333336e00,
        0x3333331e0c00,
        0x636b7f7f3600,
        0x63361c366300,
        0x3333333e301f,
        0x3f190c263f00
};

const size_t font_letter_count= sizeof(font_letter) / sizeof(font_letter[0]);
