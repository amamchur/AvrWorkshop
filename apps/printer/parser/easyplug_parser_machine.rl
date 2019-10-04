#include "easyplug_parser.hpp"

%%{
	machine fsm_name;

	main := |*
	    '^#!A1' => {
    	    handler_(this, (int)easyplug_parse_event::command_a1);
        };
        any;
	*|;
}%%

namespace {
%% write data;
}

void easyplug_parse_machine::init() {
    %% write init;
}

const char* easyplug_parse_machine::do_parse(const char *p, const char *pe) {
    %% write exec;
    return p;
}
