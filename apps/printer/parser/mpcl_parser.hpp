#ifndef CMD_LINE_PARSER_HPP
#define CMD_LINE_PARSER_HPP

#include <zoal/parser/ragel_scanner.hpp>

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

class mpcl_parse_machine :  public zoal::parser::scanner_machine<> {
protected:
    void init_machine();

    const char *run_machine(const char *p, const char *pe);
};

template<size_t BufferSize>
class mpcl_parser : public zoal::parser::ragel_scanner<mpcl_parse_machine, BufferSize> {
};

#endif
