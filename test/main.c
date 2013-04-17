#include "stdio.h"
#include "malloc.h"
#include "string.h"

#include "../cssdom.h"

css_ruleset *mainCSS;

int conf_get_px(const char* select, const char* attr, int def_value) {

	printf("Staring search\n");

	css_target *trg = css_target_parse(select);

	css_rule *rule = css_find(mainCSS, trg);

	if (rule) {
		char gadlos[1024];
		stash_get(rule->apps, attr, strlen(attr)+1, gadlos);
		return atoi(gadlos);
	}

	return def_value;	
}


int main() {

	dom_object root = { NULL, NULL, NULL, NULL };

	css_ruleset *style = css_load_file("config.css");
	mainCSS = style;

	css_fprintf(stdout, style);
	
	//dom_object *nobj = dom_create("unit", "raj_ahten", "giant");
	//dom_add_child(&root, nobj);

	dom_object *all = dom_create("buildings", "", "");
	dom_object *one = dom_create("building", "aviary", "human flying");
	dom_object *two = dom_create("building", "stables", "human horse");
	dom_object *tri = dom_create("building", "guardtower", "human");
	dom_object *win = dom_create("window", "", "human");
	dom_object *dor = dom_create("door", "", "human");
	

	dom_add_child(all, one);
	dom_add_child(all, two);
	dom_add_child(all, tri);
	//dom_add_child(one, win);
	//dom_add_child(one, dor);
	dom_add_child(&root, all);

	dom_cascade(&root, style);

	int unit_width = conf_get_px("*#aviary", "birds", 12);
	printf("GOT : %d\n", unit_width);

	dom_fprintf(stdout, all);
	
	//inspect(win);
//	inspect(dor);
}
	