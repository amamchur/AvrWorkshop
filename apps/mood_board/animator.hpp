#ifndef AVR_WORKSHOP_ANIMATOR_HPP
#define AVR_WORKSHOP_ANIMATOR_HPP

#include "fonts.hpp"

#include <stddef.h>
#include <stdint.h>
#include <zoal/ic/max72xx.hpp>

template<size_t Devices, class Max72xx, class Scheduler>
class animator {
public:
    typedef void (*animation_fn)(void*);

    using self_type = animator<Devices, Max72xx, Scheduler>;
    using matrix_type_1 = zoal::ic::max72xx_data<1>;
    using matrix_type_m = zoal::ic::max72xx_data<Devices>;
    using matrix_type_f = zoal::ic::max72xx_data<Devices + 1>;

    explicit animator(Scheduler &scheduler)
        : scheduler_(scheduler) {
    }

    void message(const char *msg) {
        strcpy(message_, msg);
    }

    const char * message(const char *msg) const {
        return message_;
    }

    void animation(int anim) {
        scheduler_.clear(current_, this);
        switch (anim) {
            case 1:
                start(slide_right_wrapper);
                break;
            default:
                start(slide_left_wrapper);
                break;
        }
    }

    int animation_delay() const {
        return animation_delay_;
    }

    void animation_delay(int d) {
        if (d > 0) {
            animation_delay_ = d;
            scheduler_.clear(current_, this);
            scheduler_.schedule(0, current_, this);
        }
    }

    void slide_left() {
        auto length = strlen(message_);
        message_shift_ = message_shift_ % (length * 8);

        auto offset = message_shift_ / 8;
        auto ls = message_shift_ % 8;

        f_matrix_.clear();
        for (auto dest : f_matrix_.data) {
            auto ch = message_[offset];
            auto src = reinterpret_cast<const uint8_t *>(font_glyphs + ch - start_glyph_code);
            for (uint8_t j = 0; j < 8; j++) {
                *dest++ = pgm_read_byte(src + j);
            }

            offset = (offset + 1) % length;
        }

        f_matrix_.push_column_msb(0, ls);

        Max72xx::display(packed.m_matrix_);
        scheduler_.schedule(animation_delay_, slide_left_wrapper, this);
        message_shift_++;
    }

    void slide_right() {
        auto length = strlen(message_);
        message_shift_ = message_shift_ % (length * 8);

        auto offset = length - message_shift_ / 8 - 1;
        auto ls = message_shift_ % 8;

        f_matrix_.clear();
        for (auto dest : f_matrix_.data) {
            auto ch = message_[offset];
            auto src = reinterpret_cast<const uint8_t *>(font_glyphs + ch - start_glyph_code);
            for (uint8_t j = 0; j < 8; j++) {
                *dest++ = pgm_read_byte(src + j);
            }

            offset = (offset + 1) % length;
        }

        f_matrix_.push_column_lsb(0, ls);

        Max72xx::display(packed.m_matrix_);
        scheduler_.schedule(animation_delay_, slide_right_wrapper, this);
        message_shift_++;
    }

    void start(animation_fn fn = &self_type::slide_right_wrapper) {
        current_ = fn;
        scheduler_.schedule(0, fn, this);
    }

private:

    static void slide_left_wrapper(void *ptr) {
        reinterpret_cast<self_type *>(ptr)->slide_left();
    }

    static void slide_right_wrapper(void *ptr) {
        reinterpret_cast<self_type *>(ptr)->slide_right();
    }

    animation_fn current_{nullptr};
    char message_[64]{0};
    int message_shift_{0};
    int animation_delay_{50};
    Scheduler &scheduler_;

    union {
        struct {
            matrix_type_1 l_matrix_;
            matrix_type_m m_matrix_;
            matrix_type_1 r_matrix_;
        } packed;
        matrix_type_f f_matrix_;
    };
};

#endif
