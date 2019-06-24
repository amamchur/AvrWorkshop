#include "parser.hpp"

void base_parser::empty_callback(base_parser *p, parse_event e) {
}

void base_parser::quoted_param_found_action() {
    ts++;
    te--;

    auto dst = const_cast<char *>(ts);
    for (auto ptr = ts; ptr != te; ptr++) {
        if (*ptr != '\\') {
            *dst++ = *ptr;
            continue;
        }

        switch (*++ptr) {
            case 'n':
                *dst = '\n';
                break;
            case 'r':
                *dst = '\r';
                break;
            case 't':
                *dst = '\t';
                break;
            case 'v':
                *dst = '\v';
                break;
            case 'f':
                *dst = '\f';
                break;
            default:
                *dst = *ptr;
                break;
        }

        dst++;
    }

    te = dst;

    if (handler_) {
        handler_(this, parse_event::string_token);
    }
}

void base_parser::push(char ch) {
    if (length_ == size_) {
        length_ = 0;
    }

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


void base_parser::parse() {
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
