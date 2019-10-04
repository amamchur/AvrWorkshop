//
// Created by amam on 04.10.19.
//

#ifndef AVR_WORKSHOP_EASYPLUG_PARSER_HPP
#define AVR_WORKSHOP_EASYPLUG_PARSER_HPP

#include "base_parser.hpp"

enum class easyplug_parse_event {
    command_a1,
};

class easyplug_parse_machine : public base_parser {
public:
    void init() override;

    const char *do_parse(const char *p, const char *pe) override;
};

template<size_t BufferSize>
class easyplug_parser : public easyplug_parse_machine {
public:
    easyplug_parser() {
        size_ = BufferSize;
    }
private:
    char ext_buffer_[BufferSize]{0};
};

#endif

