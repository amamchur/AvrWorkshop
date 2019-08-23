
#line 1 "parser_machine.rl"
#include "parser.hpp"


#line 33 "parser_machine.rl"


namespace {

#line 12 "parser_machine.cpp"
static const char _fsm_name_to_state_actions[] = {
	0, 0, 9, 0
};

static const char _fsm_name_from_state_actions[] = {
	0, 0, 10, 0
};

static const int fsm_name_start = 2;
static const int fsm_name_first_final = 2;
static const int fsm_name_error = -1;

static const int fsm_name_en_main = 2;


#line 37 "parser_machine.rl"
}

void base_parser::init() {
    
#line 33 "parser_machine.cpp"
	{
	cs = fsm_name_start;
	ts = 0;
	te = 0;
	act = 0;
	}

#line 41 "parser_machine.rl"
}

const char* base_parser::do_parse(const char *p, const char *pe) {
    
#line 46 "parser_machine.cpp"
	{
	if ( p == pe )
		goto _test_eof;
_resume:
	switch ( _fsm_name_from_state_actions[cs] ) {
	case 10:
#line 1 "NONE"
	{ts = p;}
	break;
#line 56 "parser_machine.cpp"
	}

	switch ( cs ) {
case 2:
	switch( (*p) ) {
		case 5: goto tr10;
		case 94: goto tr11;
	}
	goto tr9;
case 3:
	if ( (*p) == 77 )
		goto tr13;
	goto tr12;
case 0:
	switch( (*p) ) {
		case 67: goto tr1;
		case 73: goto tr2;
		case 77: goto tr3;
		case 80: goto tr4;
		case 82: goto tr5;
		case 84: goto tr6;
		case 86: goto tr7;
	}
	goto tr0;
case 1:
	if ( (*p) == 83 )
		goto tr8;
	goto tr0;
	}

	tr13: cs = 0; goto _again;
	tr6: cs = 1; goto _again;
	tr0: cs = 2; goto f0;
	tr1: cs = 2; goto f1;
	tr2: cs = 2; goto f2;
	tr3: cs = 2; goto f3;
	tr4: cs = 2; goto f4;
	tr5: cs = 2; goto f5;
	tr7: cs = 2; goto f6;
	tr8: cs = 2; goto f7;
	tr9: cs = 2; goto f10;
	tr10: cs = 2; goto f11;
	tr12: cs = 2; goto f13;
	tr11: cs = 3; goto f12;

f12:
#line 1 "NONE"
	{te = p+1;}
	goto _again;
f3:
#line 7 "parser_machine.rl"
	{te = p+1;{
    	    handler_(this, parse_event::command_mm);
        }}
	goto _again;
f6:
#line 10 "parser_machine.rl"
	{te = p+1;{
            handler_(this, parse_event::command_mv);
        }}
	goto _again;
f5:
#line 13 "parser_machine.rl"
	{te = p+1;{
            handler_(this, parse_event::command_mr);
        }}
	goto _again;
f1:
#line 16 "parser_machine.rl"
	{te = p+1;{
            handler_(this, parse_event::command_mc);
        }}
	goto _again;
f2:
#line 19 "parser_machine.rl"
	{te = p+1;{
            handler_(this, parse_event::command_mi);
        }}
	goto _again;
f4:
#line 22 "parser_machine.rl"
	{te = p+1;{
            handler_(this, parse_event::command_mp);
        }}
	goto _again;
f7:
#line 25 "parser_machine.rl"
	{te = p+1;{
            handler_(this, parse_event::command_mts);
        }}
	goto _again;
f11:
#line 28 "parser_machine.rl"
	{te = p+1;{
            handler_(this, parse_event::command_enq);
        }}
	goto _again;
f10:
#line 31 "parser_machine.rl"
	{te = p+1;}
	goto _again;
f13:
#line 31 "parser_machine.rl"
	{te = p;p--;}
	goto _again;
f0:
#line 31 "parser_machine.rl"
	{{p = ((te))-1;}}
	goto _again;

_again:
	switch ( _fsm_name_to_state_actions[cs] ) {
	case 9:
#line 1 "NONE"
	{ts = 0;}
	break;
#line 173 "parser_machine.cpp"
	}

	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 3: goto tr12;
	case 0: goto tr0;
	case 1: goto tr0;
	}
	}

	}

#line 45 "parser_machine.rl"
    return p;
}
