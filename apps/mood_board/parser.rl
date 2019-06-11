#include "parser.hpp"

%%{
	machine fsm_name;

	quoted_param = '"' ((ascii - cntrl - space - '\\') | [ \t] | '\\'["tvfnr])+ '"';

	main := |*
	    'msg' => {
    	    handler_(this, parse_event::command_msg);
        };
        'on' => {
            handler_(this, parse_event::command_on);
        };
        'off' => {
            handler_(this, parse_event::command_off);
        };
        'delay' => {
            handler_(this, parse_event::command_delay);
        };
        'intensity' => {
            handler_(this, parse_event::command_intensity);
        };
        'animation' => {
            handler_(this, parse_event::command_animation);
        };
        [\+\-]? digit+ =>{
            handler_(this, parse_event::number_token);
        };
        quoted_param => {
            quoted_param_found_action();
        };
        32;
        '\n' => {
            handler_(this, parse_event::line_end);
        };

        any;
	*|;
}%%

namespace {
%% write data;
}

void parser::quoted_param_found_action() {
    ts++;
    te--;

    auto dst = const_cast<char *>(ts);
    for (auto ptr = ts; ptr != te; ptr++) {
        if (*ptr != '\\') {
            *dst++ = *ptr;
            continue;
        }

        switch (*++ptr) {
            case 'n':
                *dst = '\n';
                break;
            case 'r':
                *dst = '\r';
                break;
            case 't':
                *dst = '\t';
                break;
            case 'v':
                *dst = '\v';
                break;
            case 'f':
                *dst = '\f';
                break;
            default:
                *dst = *ptr;
                break;
        }

        dst++;
    }

    te = dst;

    if (handler_) {
        handler_(this, parse_event::string_token);
    }
}

void parser::init() {
    %% write init;
}

const char* parser::do_parse(const char *p, const char *pe) {
    %% write exec;
    return p;
}

void parser::empty_callback(parser *p, parse_event e) {

}
