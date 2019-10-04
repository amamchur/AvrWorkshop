#ifndef CMD_LINE_PARSER_HPP
#define CMD_LINE_PARSER_HPP

#include "base_parser.hpp"

enum class mpcl_parse_event {
    command_enq,
    command_mm,
    command_mv,
    command_mr,
    command_mc,
    command_mi,
    command_mp,
    command_mts
};

class mpcl_parse_machine : public base_parser {
public:
    void init() override;

    const char *do_parse(const char *p, const char *pe) override;
};

template<size_t BufferSize>
class mpcl_parser : public mpcl_parse_machine {
public:
    mpcl_parser() {
        size_ = BufferSize;
    }
private:
    char ext_buffer_[BufferSize]{0};
};

#endif
