#include "easyplug_parser.hpp"

%%{
	machine easyplug_scanner;

	main := |*
	    '^#!A1' => {
    	    handler_(this, (int)easyplug_parse_event::command_a1);
        };
        '#!X0' => {
            handler_(this, (int)easyplug_parse_event::command_x0);
        };
        '#!PG-1#G' => {
            handler_(this, (int)easyplug_parse_event::command_pg1);
        };
        '#!PG30068#G' => {
             handler_(this, (int)easyplug_parse_event::command_pg30068);
         };
        '#!PG30021#G' => {
              handler_(this, (int)easyplug_parse_event::command_pg30021);
        };
        any;
	*|;
}%%

namespace {
%% write data;
}

void easyplug_parse_machine::init_machine() {
    %% write init;
}

const char* easyplug_parse_machine::run_machine(const char *p, const char *pe) {
    %% write exec;
    return p;
}
