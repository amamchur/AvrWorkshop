#include "mpcl_parser.hpp"

%%{
	machine fsm_name;

	main := |*
	    '^MM' => {
    	    handler_(this, (int)mpcl_parse_event::command_mm);
        };
        '^MV' => {
            handler_(this, (int)mpcl_parse_event::command_mv);
        };
        '^MR' => {
            handler_(this, (int)mpcl_parse_event::command_mr);
        };
        '^MC' => {
            handler_(this, (int)mpcl_parse_event::command_mc);
        };
        '^MI' => {
            handler_(this, (int)mpcl_parse_event::command_mi);
        };
        '^MP' => {
            handler_(this, (int)mpcl_parse_event::command_mp);
        };
        '^MTS' =>{
            handler_(this, (int)mpcl_parse_event::command_mts);
        };
        5 =>{
            handler_(this, (int)mpcl_parse_event::command_enq);
        };
        any;
	*|;
}%%

namespace {
%% write data;
}

void mpcl_parse_machine::init() {
    %% write init;
}

const char* mpcl_parse_machine::do_parse(const char *p, const char *pe) {
    %% write exec;
    return p;
}
