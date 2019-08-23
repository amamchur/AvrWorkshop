#include "parser.hpp"

void base_parser::empty_callback(base_parser *p, parse_event e) {
}

void base_parser::push(char ch) {
    if (length_ == size_) {
        length_ = 0;
    }

    buffer_[length_++] = ch;
    parse();
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
