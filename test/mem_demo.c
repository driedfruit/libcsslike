#include "stdio.h"
#include "malloc.h"
#include "string.h"

#include "../cssdom.h"

const char css[] = 
"body {" "\n"
"	background: red;" "\n"
"}" "\n"
"div {" "\n"
"	display: block;" "\n"
"}" "\n"
;

int main() {

	css_ruleset *style;// = css_load_file("config.css");
	css_parser *state = css_parser_create();

	css_parse(state, css, sizeof(css));

	style = css_parser_done(state);

	css_fprintf(stdout, style);

	css_free(style);
}