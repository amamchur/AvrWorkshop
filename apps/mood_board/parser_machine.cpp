
/* #line 1 "./apps/mood_board/mpcl_parser_machine.rl" */
#include "parser.hpp"


/* #line 40 "./apps/mood_board/mpcl_parser_machine.rl" */


namespace {

/* #line 12 "./apps/mood_board/parser_machine.cpp" */
static const int fsm_name_start = 21;
static const int fsm_name_first_final = 21;
static const int fsm_name_error = -1;

static const int fsm_name_en_main = 21;


/* #line 44 "./apps/mood_board/mpcl_parser_machine.rl" */
}

void base_parser::init() {
    
/* #line 25 "./apps/mood_board/parser_machine.cpp" */
	{
	cs = fsm_name_start;
	ts = 0;
	te = 0;
	act = 0;
	}

/* #line 48 "./apps/mood_board/mpcl_parser_machine.rl" */
}

const char* base_parser::do_parse(const char *p, const char *pe) {
    
/* #line 38 "./apps/mood_board/parser_machine.cpp" */
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
tr0:
/* #line 1 "NONE" */
	{	switch( act ) {
	case 7:
	{{p = ((te))-1;}
            handler_(this, mpcl_parse_event::number_token);
        }
	break;
	case 8:
	{{p = ((te))-1;}
            quoted_param_found_action();
        }
	break;
	default:
	{{p = ((te))-1;}}
	break;
	}
	}
	goto st21;
tr4:
/* #line 38 "./apps/mood_board/mpcl_parser_machine.rl" */
	{{p = ((te))-1;}}
	goto st21;
tr11:
/* #line 24 "./apps/mood_board/mpcl_parser_machine.rl" */
	{te = p+1;{
            handler_(this, mpcl_parse_event::command_animation);
        }}
	goto st21;
tr14:
/* #line 18 "./apps/mood_board/mpcl_parser_machine.rl" */
	{te = p+1;{
            handler_(this, mpcl_parse_event::command_delay);
        }}
	goto st21;
tr21:
/* #line 21 "./apps/mood_board/mpcl_parser_machine.rl" */
	{te = p+1;{
            handler_(this, mpcl_parse_event::command_intensity);
        }}
	goto st21;
tr22:
/* #line 9 "./apps/mood_board/mpcl_parser_machine.rl" */
	{te = p+1;{
    	    handler_(this, mpcl_parse_event::command_msg);
        }}
	goto st21;
tr23:
/* #line 15 "./apps/mood_board/mpcl_parser_machine.rl" */
	{te = p+1;{
            handler_(this, mpcl_parse_event::command_off);
        }}
	goto st21;
tr24:
/* #line 38 "./apps/mood_board/mpcl_parser_machine.rl" */
	{te = p+1;}
	goto st21;
tr25:
/* #line 34 "./apps/mood_board/mpcl_parser_machine.rl" */
	{te = p+1;{
            handler_(this, mpcl_parse_event::line_end);
        }}
	goto st21;
tr26:
/* #line 33 "./apps/mood_board/mpcl_parser_machine.rl" */
	{te = p+1;}
	goto st21;
tr35:
/* #line 38 "./apps/mood_board/mpcl_parser_machine.rl" */
	{te = p;p--;}
	goto st21;
tr36:
/* #line 30 "./apps/mood_board/mpcl_parser_machine.rl" */
	{te = p;p--;{
            quoted_param_found_action();
        }}
	goto st21;
tr42:
/* #line 12 "./apps/mood_board/mpcl_parser_machine.rl" */
	{te = p+1;{
            handler_(this, mpcl_parse_event::command_on);
        }}
	goto st21;
st21:
/* #line 1 "NONE" */
	{ts = 0;}
	if ( ++p == pe )
		goto _test_eof21;
case 21:
/* #line 1 "NONE" */
	{ts = p;}
/* #line 135 "./apps/mood_board/parser_machine.cpp" */
	switch( (*p) ) {
		case 10: goto tr25;
		case 32: goto tr26;
		case 34: goto tr27;
		case 43: goto tr28;
		case 45: goto tr28;
		case 97: goto tr30;
		case 100: goto tr31;
		case 105: goto tr32;
		case 109: goto tr33;
		case 111: goto tr34;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr29;
	goto tr24;
tr27:
/* #line 1 "NONE" */
	{te = p+1;}
/* #line 38 "./apps/mood_board/mpcl_parser_machine.rl" */
	{act = 11;}
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
/* #line 161 "./apps/mood_board/parser_machine.cpp" */
	switch( (*p) ) {
		case 9: goto st0;
		case 92: goto st1;
	}
	if ( 32 <= (*p) && (*p) <= 126 )
		goto st0;
	goto tr35;
st0:
	if ( ++p == pe )
		goto _test_eof0;
case 0:
	switch( (*p) ) {
		case 9: goto st0;
		case 34: goto tr2;
		case 92: goto st1;
	}
	if ( 32 <= (*p) && (*p) <= 126 )
		goto st0;
	goto tr0;
tr2:
/* #line 1 "NONE" */
	{te = p+1;}
/* #line 30 "./apps/mood_board/mpcl_parser_machine.rl" */
	{act = 8;}
	goto st23;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
/* #line 191 "./apps/mood_board/parser_machine.cpp" */
	switch( (*p) ) {
		case 9: goto st0;
		case 34: goto tr2;
		case 92: goto st1;
	}
	if ( 32 <= (*p) && (*p) <= 126 )
		goto st0;
	goto tr36;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
	switch( (*p) ) {
		case 34: goto st0;
		case 102: goto st0;
		case 110: goto st0;
		case 114: goto st0;
		case 116: goto st0;
		case 118: goto st0;
	}
	goto tr0;
tr28:
/* #line 1 "NONE" */
	{te = p+1;}
/* #line 38 "./apps/mood_board/mpcl_parser_machine.rl" */
	{act = 11;}
	goto st24;
tr29:
/* #line 1 "NONE" */
	{te = p+1;}
/* #line 27 "./apps/mood_board/mpcl_parser_machine.rl" */
	{act = 7;}
	goto st24;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
/* #line 229 "./apps/mood_board/parser_machine.cpp" */
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr29;
	goto tr0;
tr30:
/* #line 1 "NONE" */
	{te = p+1;}
	goto st25;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
/* #line 241 "./apps/mood_board/parser_machine.cpp" */
	if ( (*p) == 110 )
		goto st2;
	goto tr35;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	if ( (*p) == 105 )
		goto st3;
	goto tr4;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	if ( (*p) == 109 )
		goto st4;
	goto tr4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	if ( (*p) == 97 )
		goto st5;
	goto tr4;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	if ( (*p) == 116 )
		goto st6;
	goto tr4;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( (*p) == 105 )
		goto st7;
	goto tr4;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	if ( (*p) == 111 )
		goto st8;
	goto tr4;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	if ( (*p) == 110 )
		goto tr11;
	goto tr4;
tr31:
/* #line 1 "NONE" */
	{te = p+1;}
	goto st26;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
/* #line 302 "./apps/mood_board/parser_machine.cpp" */
	if ( (*p) == 101 )
		goto st9;
	goto tr35;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( (*p) == 108 )
		goto st10;
	goto tr4;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	if ( (*p) == 97 )
		goto st11;
	goto tr4;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	if ( (*p) == 121 )
		goto tr14;
	goto tr4;
tr32:
/* #line 1 "NONE" */
	{te = p+1;}
	goto st27;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
/* #line 335 "./apps/mood_board/parser_machine.cpp" */
	if ( (*p) == 110 )
		goto st12;
	goto tr35;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	if ( (*p) == 116 )
		goto st13;
	goto tr4;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	if ( (*p) == 101 )
		goto st14;
	goto tr4;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	if ( (*p) == 110 )
		goto st15;
	goto tr4;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( (*p) == 115 )
		goto st16;
	goto tr4;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	if ( (*p) == 105 )
		goto st17;
	goto tr4;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	if ( (*p) == 116 )
		goto st18;
	goto tr4;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	if ( (*p) == 121 )
		goto tr21;
	goto tr4;
tr33:
/* #line 1 "NONE" */
	{te = p+1;}
	goto st28;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
/* #line 396 "./apps/mood_board/parser_machine.cpp" */
	if ( (*p) == 115 )
		goto st19;
	goto tr35;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	if ( (*p) == 103 )
		goto tr22;
	goto tr4;
tr34:
/* #line 1 "NONE" */
	{te = p+1;}
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
/* #line 415 "./apps/mood_board/parser_machine.cpp" */
	switch( (*p) ) {
		case 102: goto st20;
		case 110: goto tr42;
	}
	goto tr35;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	if ( (*p) == 102 )
		goto tr23;
	goto tr4;
	}
	_test_eof21: cs = 21; goto _test_eof; 
	_test_eof22: cs = 22; goto _test_eof; 
	_test_eof0: cs = 0; goto _test_eof; 
	_test_eof23: cs = 23; goto _test_eof; 
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof24: cs = 24; goto _test_eof; 
	_test_eof25: cs = 25; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof26: cs = 26; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof27: cs = 27; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof28: cs = 28; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 
	_test_eof29: cs = 29; goto _test_eof; 
	_test_eof20: cs = 20; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 22: goto tr35;
	case 0: goto tr0;
	case 23: goto tr36;
	case 1: goto tr0;
	case 24: goto tr0;
	case 25: goto tr35;
	case 2: goto tr4;
	case 3: goto tr4;
	case 4: goto tr4;
	case 5: goto tr4;
	case 6: goto tr4;
	case 7: goto tr4;
	case 8: goto tr4;
	case 26: goto tr35;
	case 9: goto tr4;
	case 10: goto tr4;
	case 11: goto tr4;
	case 27: goto tr35;
	case 12: goto tr4;
	case 13: goto tr4;
	case 14: goto tr4;
	case 15: goto tr4;
	case 16: goto tr4;
	case 17: goto tr4;
	case 18: goto tr4;
	case 28: goto tr35;
	case 19: goto tr4;
	case 29: goto tr35;
	case 20: goto tr4;
	}
	}

	}

/* #line 52 "./apps/mood_board/mpcl_parser_machine.rl" */
    return p;
}
