#ifndef CMD_LINE_PARSER_HPP
#define CMD_LINE_PARSER_HPP

#include <stdint.h>
#include <stddef.h>

enum class parse_event {
    command_error,
    command_unknown,
    command_msg,
    command_on,
    command_off,
    command_delay,
    command_intensity,
    command_animation,
    string_token,
    number_token,
    line_end
};

class parser {
public:

    typedef void (*callback_fn)(parser *p, parse_event e);

    void push(char ch) {
        buffer_[length_++] = ch;

        switch (ch) {
            case ' ':
            case '\r':
            case '\n':
            case '"':
                parse();
                break;
            default:
                break;
        }
    }

    void parse() {
        init();
        do_parse(buffer_, buffer_ + length_);
        if (ts == nullptr) {
            length_ = 0;
            return;
        }

        auto end = buffer_ + length_;
        auto dst = buffer_;
        for (auto ptr = ts; ptr != end;) {
            *dst++ = *ptr++;
        }

        te = buffer_ + (te - ts);
        ts = buffer_;
        length_ = dst - buffer_;
    }

    const char* do_parse(char const *p, const char *pe);

    inline void context(void *c) {
        this->context_ = c;
    }

    inline void *context() const {
        return context_;
    }

    inline void callback(callback_fn cb) {
        this->handler_ = cb;
    }

    inline callback_fn callback() const {
        return handler_;
    }

    inline const char *token_start() const {
        return ts;
    }

    inline const char *token_end() const {
        return te;
    }
//private:

    void init();

    void quoted_param_found_action();

    static void empty_callback(parser *p, parse_event e);

    void *context_{nullptr};
    callback_fn handler_{&empty_callback};

    int cs{0};
    int act{0};
    const char *ts{nullptr};
    const char *te{nullptr};
    const char *eof{nullptr};

    size_t size_{512};
    size_t length_{0};
    char buffer_[512]{0};

};

#endif
