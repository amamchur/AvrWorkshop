#ifndef CMD_LINE_PARSER_HPP
#define CMD_LINE_PARSER_HPP

#include <stddef.h>
#include <stdint.h>

enum class mpcl_parse_event {
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

class base_parser {
public:
    typedef void (*callback_fn)(base_parser *p, mpcl_parse_event e);

    void push(char ch);

    void parse();

    const char *do_parse(char const *p, const char *pe);

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

protected:
    void init();

    void quoted_param_found_action();

    static void empty_callback(base_parser *p, mpcl_parse_event e);

    void *context_{nullptr};
    callback_fn handler_{&empty_callback};

    int cs{0};
    int act{0};
    const char *ts{nullptr};
    const char *te{nullptr};
    const char *eof{nullptr};

    size_t size_{0};
    size_t length_{0};
    char buffer_[0];
};

template<size_t BufferSize>
class mpcl_parser : public base_parser {
public:
    mpcl_parser() {
        size_ = BufferSize;
    }

private:
    char ext_buffer_[BufferSize]{0};
};

#endif
