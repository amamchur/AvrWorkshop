#include <zoal/parser/ragel_scanner.hpp>

enum class easyplug_parse_event {
    command_a1,
    command_x0,
    command_pg1,
    command_pg30021,
    command_pg30068
};

class easyplug_parse_machine : public zoal::parser::scanner_machine<> {
public:
    void init_machine();

    const char *run_machine(const char *p, const char *pe);
};

template<size_t BufferSize>
class easyplug_parser : public zoal::parser::ragel_scanner<easyplug_parse_machine, BufferSize> {
};


easyplug_parser<128> parser;

int main() {
    char test[] = "#!PG-1#G";
    char *p = test;
    while (*p) {
        parser.push(p++, 1);
    }
    return 0;
}
