#ifndef AVR_WORKSHOP_BASE_PARSER_HPP
#define AVR_WORKSHOP_BASE_PARSER_HPP

#include <stdint.h>
#include <stddef.h>

class base_parser {
public:
    typedef void (*callback_fn)(base_parser *p, int e);

    void push(char ch);

    void parse();

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

    virtual void init() = 0;
    virtual const char *do_parse(char const *p, const char *pe) = 0;
protected:

    static void empty_callback(base_parser *p, int e);

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

#endif
