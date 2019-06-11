#ifndef AVR_WORKSHOP_COMMAND_HPP
#define AVR_WORKSHOP_COMMAND_HPP

#include "parser.hpp"

template<size_t Devices, class Max72xx, class Animator, class Logger>
class executer {
public:
    explicit executer(Animator &animator)
        : animator(animator) {}

    void handle(parser *p, parse_event event) {
        (*this.*current)(p, event);
    }

private:
    typedef void (executer::*handle_fn)(parser *p, parse_event event);

    void flush_delay(parser *p, parse_event event) {
        switch (event) {
        case parse_event::line_end:
            animator.animation_delay(number_token);
            current = &executer::select_command;
            break;
        default:
            current = &executer::skip_until_end_line;
            break;
        }
    }

    int token_to_int(parser *p) {
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

    void take_delay(parser *p, parse_event event) {
        switch (event) {
        case parse_event::number_token: {
            number_token = token_to_int(p);
            current = &executer::flush_delay;
            break;
        }
        default:
            current = &executer::skip_until_end_line;
            break;
        }
    }

    void flush_intensity(parser *p, parse_event event) {
        switch (event) {
        case parse_event::line_end: {
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

    void take_intensity(parser *p, parse_event event) {
        switch (event) {
        case parse_event::number_token: {
            number_token = token_to_int(p);
            current = &executer::flush_intensity;
            break;
        }
        default:
            current = &executer::skip_until_end_line;
            break;
        }
    }

    void flush_animation(parser *p, parse_event event) {
        switch (event) {
            case parse_event::line_end: {
                typename Logger::info() << "animation: " << number_token;
                animator.animation(number_token);
                current = &executer::select_command;
                break;
            }
            default:
                current = &executer::skip_until_end_line;
                break;
        }
    }

    void take_animation(parser *p, parse_event event) {
        switch (event) {
            case parse_event::number_token: {
                number_token = token_to_int(p);
                current = &executer::flush_animation;
                break;
            }
            default:
                current = &executer::skip_until_end_line;
                break;
        }
    }

    void select_command(parser *p, parse_event event) {
        switch (event) {
        case parse_event::command_msg:
            current = &executer::take_message;
            break;
        case parse_event::command_on:
            current = &executer::display_on;
            break;
        case parse_event::command_off:
            current = &executer::display_off;
            break;
        case parse_event::command_delay:
            current = &executer::take_delay;
            break;
        case parse_event::command_intensity:
            current = &executer::take_intensity;
            break;
        case parse_event::command_animation:
            current = &executer::take_animation;
            break;
        case parse_event::line_end:
            break;
        default:
            current = &executer::skip_until_end_line;
            break;
        }
    }

    void take_message(parser *p, parse_event event) {
        switch (event) {
        case parse_event::string_token: {
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

    void skip_until_end_line(parser *p, parse_event event) {
        switch (event) {
        case parse_event::line_end:
            current = &executer::select_command;
            break;
        default:
            break;
        }
    }

    void flush_message(parser *p, parse_event event) {
        switch (event) {
        case parse_event::line_end:
            animator.message(string_token);
            current = &executer::select_command;
            break;
        default:
            current = &executer::skip_until_end_line;
            break;
        }
    }

    void display_on(parser *p, parse_event event) {
        switch (event) {
        case parse_event::line_end:
            Max72xx::send(Devices, Max72xx::on);
            current = &executer::select_command;
            break;
        default:
            current = &executer::skip_until_end_line;
            break;
        }
    }

    void display_off(parser *p, parse_event event) {
        switch (event) {
        case parse_event::line_end:
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
    char string_token[64]{0};
    int number_token{0};
};

#endif
