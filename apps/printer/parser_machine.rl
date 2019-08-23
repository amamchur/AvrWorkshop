#include "parser.hpp"

%%{
	machine fsm_name;

	main := |*
	    '^MM' => {
    	    handler_(this, parse_event::command_mm);
        };
        '^MV' => {
            handler_(this, parse_event::command_mv);
        };
        '^MR' => {
            handler_(this, parse_event::command_mr);
        };
        '^MC' => {
            handler_(this, parse_event::command_mc);
        };
        '^MI' => {
            handler_(this, parse_event::command_mi);
        };
        '^MP' => {
            handler_(this, parse_event::command_mp);
        };
        '^MTS' =>{
            handler_(this, parse_event::command_mts);
        };
        5 =>{
            handler_(this, parse_event::command_enq);
        };
        any;
	*|;
}%%

namespace {
%% write data;
}

void base_parser::init() {
    %% write init;
}

const char* base_parser::do_parse(const char *p, const char *pe) {
    %% write exec;
    return p;
}
