#ifndef AVR_WORKSHOP_COMMAND_HPP
#define AVR_WORKSHOP_COMMAND_HPP

#include "parser.hpp"

template<size_t MsgLength, size_t Devices, class Max72xx, class Animator, class Logger>
class executer {
public:
    explicit executer(Animator &animator)
        : animator(animator) {}

    void handle(ragel_scanner *p, mpcl_parse_event event) {
        (*this.*current)(p, event);
    }

private:
    typedef void (executer::*handle_fn)(ragel_scanner *p, mpcl_parse_event event);

    void flush_delay(ragel_scanner *p, mpcl_parse_event event) {
        switch (event) {
        case mpcl_parse_event::line_end:
            animator.animation_delay(number_token);
            current = &executer::select_command;
            break;
        default:
            current = &executer::skip_until_end_line;
            break;
        }
    }

    int token_to_int(ragel_scanner *p) {
        int value = 0;
        int sign = 1;
        auto s = p->token_start();
        auto e = p->token_end();
        for (; s != e; s++) {
            if (*s == '-') {
                sign -= 1;
            }

            value = value * 10 + (*s - '0');
        }

        return value * sign;
    }

    void take_delay(ragel_scanner *p, mpcl_parse_event event) {
        switch (event) {
        case mpcl_parse_event::number_token: {
            number_token = token_to_int(p);
            current = &executer::flush_delay;
            break;
        }
        default:
            current = &executer::skip_until_end_line;
            break;
        }
    }

    void flush_intensity(ragel_scanner *p, mpcl_parse_event event) {
        switch (event) {
        case mpcl_parse_event::line_end: {
            auto value = (uint8_t)number_token & 0xFu;
            Max72xx::send(Devices, Max72xx::intensity0 | value);
            current = &executer::select_command;
            break;
        }
        default:
            current = &executer::skip_until_end_line;
            break;
        }
    }

    void take_intensity(ragel_scanner *p, mpcl_parse_event event) {
        switch (event) {
        case mpcl_parse_event::number_token: {
            number_token = token_to_int(p);
            current = &executer::flush_intensity;
            break;
        }
        default:
            current = &executer::skip_until_end_line;
            break;
        }
    }

    void flush_animation(ragel_scanner *p, mpcl_parse_event event) {
        switch (event) {
            case mpcl_parse_event::line_end: {
                Logger::info() << "animation: " << number_token;
                animator.animation(number_token);
                current = &executer::select_command;
                break;
            }
            default:
                current = &executer::skip_until_end_line;
                break;
        }
    }

    void take_animation(ragel_scanner *p, mpcl_parse_event event) {
        switch (event) {
            case mpcl_parse_event::number_token: {
                number_token = token_to_int(p);
                current = &executer::flush_animation;
                break;
            }
            default:
                current = &executer::skip_until_end_line;
                break;
        }
    }

    void select_command(ragel_scanner *p, mpcl_parse_event event) {
        switch (event) {
        case mpcl_parse_event::command_msg:
            current = &executer::take_message;
            break;
        case mpcl_parse_event::command_on:
            current = &executer::display_on;
            break;
        case mpcl_parse_event::command_off:
            current = &executer::display_off;
            break;
        case mpcl_parse_event::command_delay:
            current = &executer::take_delay;
            break;
        case mpcl_parse_event::command_intensity:
            current = &executer::take_intensity;
            break;
        case mpcl_parse_event::command_animation:
            current = &executer::take_animation;
            break;
        case mpcl_parse_event::line_end:
            break;
        default:
            current = &executer::skip_until_end_line;
            break;
        }
    }

    void take_message(ragel_scanner *p, mpcl_parse_event event) {
        switch (event) {
        case mpcl_parse_event::string_token: {
            auto s = p->token_start();
            auto e = p->token_end();
            auto result = copy_token(s, e);
            current = result ? &executer::flush_message : &executer::skip_until_end_line;
            break;
        }
        default:
            current = &executer::skip_until_end_line;
            break;
        }
    }

    bool copy_token(const char *s, const char *e) {
        if ((int)sizeof(string_token) - 1 < (e - s)) {
            string_token[0] = '!';
            string_token[1] = 0;
            return false;
        }

        auto *p = string_token;
        while (s != e) {
            *p++ = *s++;
        }

        *p = 0;
        return true;
    }

    void skip_until_end_line(ragel_scanner *p, mpcl_parse_event event) {
        switch (event) {
        case mpcl_parse_event::line_end:
            current = &executer::select_command;
            break;
        default:
            break;
        }
    }

    void flush_message(ragel_scanner *p, mpcl_parse_event event) {
        switch (event) {
        case mpcl_parse_event::line_end:
            animator.message(string_token);
            current = &executer::select_command;
            break;
        default:
            current = &executer::skip_until_end_line;
            break;
        }
    }

    void display_on(ragel_scanner *p, mpcl_parse_event event) {
        switch (event) {
        case mpcl_parse_event::line_end:
            Max72xx::send(Devices, Max72xx::on);
            current = &executer::select_command;
            break;
        default:
            current = &executer::skip_until_end_line;
            break;
        }
    }

    void display_off(ragel_scanner *p, mpcl_parse_event event) {
        switch (event) {
        case mpcl_parse_event::line_end:
            Max72xx::send(Devices, Max72xx::off);
            current = &executer::select_command;
            break;
        default:
            current = &executer::skip_until_end_line;
            break;
        }
    }

    Animator &animator;
    handle_fn current{&executer::select_command};
    char string_token[MsgLength]{0};
    int number_token{0};
};

#endif
