#include "printer/parser.hpp"

#include <iostream>

static void command_callback(base_parser *p, parse_event e) {
    std::cout << "command_callback: " << (int)e << std::endl;
}

int main() {
    char msg[] = "^MR^MM\x05^MTS^MI\r\n";
    parser<232> test_parser;
    test_parser.callback(command_callback);

    for (auto c = msg; *c; c++) {
        test_parser.push(*c);
    }

    return 0;
}
