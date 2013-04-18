/* 
 * Rudimentary CSS and DOM.
 */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>

#include "cssdom.h"

LL_ADD_NEXT(css_target, css_target, next);

LL_ADD_NEXT(css_rule, css_rule, next);

LLT_PACKAGE(dom, dom_object, childNode, nextSibling, parentNode);

void css_rule_add_one(css_rule *rule, const char *k, const char *v) {
	stash_put(rule->apps, k, strlen(k) + 1, H_PTR(v), strlen(v)+1);
}

css_rule* css_rule_create(css_target *target, const char *k, const char *v) {
	css_rule * rule = malloc(sizeof(css_rule));
	if (rule) {
		rule->apps = stash_new();
		if (rule->apps) {
			rule->target = target;
			if (k != NULL && v != NULL)
			    stash_put(rule->apps, k, strlen(k) + 1, H_PTR(v), strlen(v)+1);
			rule->next = NULL;			
		} else { free(rule); rule = NULL; }
	}
	return rule;
}

css_target* css_target_create(css_target *prev, const char *name, const char *value, const char type, const char rela) {
	css_target * trg = malloc(sizeof(css_target));
	if (trg) {
		strncpy(trg->name, name, sizeof(trg->name));			
		strncpy(trg->value, value, sizeof(trg->value));
		trg->type = type;
		trg->rela = rela;
		trg->next = NULL;
		if (prev) prev->next = trg;
	}
	return trg;
}

css_target* css_target_parse(const char *str) {

		//printf(" PARSING: %s\n", str);

	enum css_cascade_type {		Default, General, Direct, Sibling	};
	enum css_filter_mode  {		None, Id, Class, Attribute	};
	const char filter_name[3][6] = { "type", "id", "class" };
	const char cascade_mark[4] = { '&', ' ', '>', '+' };

	css_target *root_target = NULL;
	css_target *last_target = NULL;

	int cascade = None;
	int filter = Default;

	int tok_cascade = None, tok_filter, tok_e, tok_s;
	int tok_ended, tok_starting, tok_waiting = 0;

	const char *ptr = str; 
	while (1) {
		/* Basic reset */
		tok_starting = 0;
		tok_ended = 1;
		switch (*ptr) {
			case 0:
			case ']':
				/* Ensure tok_ended stays == 1 */ 
			break;
			case '+': cascade = Sibling; break;
			case '>': cascade = Direct; break;
			case ' ': if (cascade < General) cascade = General;	break;
			/* [, #, . - to keep the symbol add 'tok_starting = 1' to those cases: */
			case '[': filter = Attribute;	break;
			case '#': filter = Id;	break;
			case '.': filter = Class; break;
			//case ':': filter = Pseudo; break;
			default:
				/* Invert reset */
				tok_ended = 0;
				tok_starting = 1;
			break;
		}
		/* Store token */
		if (tok_ended && tok_waiting) {
			char buf[1024] = { 0 };
			strncpy(buf, &str[tok_s], tok_e);
			buf[tok_e] = '\0';

			/* Hackity-hack: */
			{
					char type = '=';
					const char* key = NULL;
					const char* value = NULL;

					if (tok_filter == Attribute) {
						int sp = strcspn(buf, "=");
						char thename[1024];
						strncpy( thename, buf, sp );
						if (sp && (thename[sp-1] == '|' || thename[sp-1] == '~')) {
							type = thename[sp-1];
							thename[sp-1] = 0;
						}
						thename[sp] = 0;
						
						char theval[1024];
						strncpy( theval, &buf[0] + sp + 1, sizeof(theval));

						key = thename;
						value = theval;	
					} else {
						if (tok_filter == Class) type = '~'; /* .class translates to [class~=] */ 
						key = filter_name[tok_filter];
						value = buf;
					}

					last_target = 
						css_target_create(last_target, key, value, type, cascade_mark[tok_cascade]);
					if (!root_target) root_target = last_target;
			}

			tok_waiting = 0;
		}
		/* Start new token */
		if (tok_starting && !tok_waiting) {
			tok_s = ptr - str;
			tok_e = 0;
			tok_waiting = 1;
			tok_filter = filter;	/* <-- */	filter = Default;
			tok_cascade = cascade; /* <-- */	cascade = None;
		}
		/* Go on */
		tok_e++; if (*ptr++ == 0) break;
	}
	return root_target;
}

css_parser* css_parser_create() {

	css_parser *state = NULL;

	state = malloc(sizeof(css_parser));
	if (state != NULL) {
		memset(state, 0, sizeof(css_parser));

		/* Point to selector buffer */
		state->tobuf = state->selector_buffer;
		state->tobuf_space = sizeof(state->selector_buffer);
	}

	return state;
}

css_ruleset* css_parser_done(css_parser *state) {

	css_ruleset *ret = state->style;

	if (state->num_rules || state->num_selectors) {
		fprintf(stderr, "Critical: Have %d unfinished rules, %d unfinished selectors!\n", state->num_rules, state->num_selectors); 
	}

	free(state);

	return ret;
}

/* TODO: add more error checks, cut corner cases */
int css_parse(css_parser *state, const char *data, int len) {

#define CHANGE_BUFFER(BUFF)	\
	state->tobuf = (BUFF); \
	*(state->tobuf) = 0; \
	state->tobuf_space = sizeof((BUFF)) 

#define OUTER_SPACE 0 /* whitespace outside the ___sel{}___ */
#define INNER_SPACE 9 /* whitespace inside the {___prop:val;___} */
#define MIDDLE_SPACE 8 /* whitespace between prop:____val */

#define COMMENT 1
#define SUBSTRING 5

#define SELECTORS 3
#define PROPERTY 6
#define VALUE 7

	int i, j;

#if 0
	printf("\n");//debug
	printf("BUF2>> %s[end]\n", state->selector_buffer);
	printf("BUF3>> %s[end]\n", state->propety_buffer);
	printf("BUF4>> %s[end]\n", state->value_buffer);
#endif

	for (i = 0; i < len; i++) {
		char c = data[i];

		//printf("[%d] -- %c\n", sector, c);
		//if ( c == '\n' || c == '\r' || c == '\t') continue;

		if (isspace(c)) {
			if (state->sector == OUTER_SPACE) continue; /* ignore outer whitespace */
			if (state->sector == PROPERTY) continue; /* ignore whitespace in property names -- note, not clean, might lead to errors */
			if (state->sector == INNER_SPACE) continue; /* ignore whitespace after property values */
			if (state->sector == MIDDLE_SPACE) continue; /* ignore whitespace inside rule blocks? */
		}

		if (c == '/' && state->sector == COMMENT) { /* end of comment */
			/* pop sector */
			state->sector = state->prev_sector;
			state->prev_sector = 0;
			continue;
		}

		if (state->sector == COMMENT) { /* inside a comment, ignore */
			continue;
		}

		if (c == '/') { /* start of comment */
			/* push sector */
			state->prev_sector = state->sector;
			state->sector = COMMENT;
			continue;
		}

		if (c == 0) break; /* not readable */

		if (c == ',') { /* end of selector? */
			if (state->sector == SELECTORS) {
				//printf("Got selector %d: `%s`\n", num_selectors, buf2);
				state->selectors[state->num_selectors++] = css_target_parse(state->selector_buffer);
				CHANGE_BUFFER(state->selector_buffer);
				state->sector = OUTER_SPACE;
				break;
			}
		}

		if (c == '{') { /* start of rule block? */
			if (state->sector == SELECTORS) {
                //printf("Got last selector: `%s`\n", buf2);
				state->selectors[state->num_selectors++] = css_target_parse(state->selector_buffer);
				CHANGE_BUFFER(state->selector_buffer);
			}
			/* now, create empty rules for each selector */
			for (j = 0; j < state->num_selectors; j++) {
				state->rules[j] = css_rule_create(state->selectors[j], NULL, NULL);
				state->num_rules++;
				if (!state->style)	state->style = state->rules[j];
				else 		css_rule_add_next(state->style, state->rules[j]);
			}
			/* and reset "pending" selectors */
			state->num_selectors = 0;

			state->sector = INNER_SPACE;
			continue;
		}
/*
		if (sector == SELECTORS) { // inside selectors, condense all spaces
			if (isspace(c))	{ extra_space = 1; continue; }
			else if (extra_space) {
				*tobuf++ = ' ';
				extra_space = 0;
			}
		}
*/
		if (c == ':' && state->sector == PROPERTY) { /* end of property name */
			state->sector = MIDDLE_SPACE;
			continue;
		}

		if (c == ';' && state->sector == VALUE) { /* end of property value */
			/* add prop/value to all pending rules */
			for (j = 0; j < state->num_rules; j++)
				css_rule_add_one(state->rules[j], state->property_buffer, strtrim(state->value_buffer));
			//printf("Added `%s=%s` to %d rule(s)\n", buf3, buf4, num_rules);

			state->sector = INNER_SPACE;
			continue;
		}

		if (c == '}') {
			if (state->sector == VALUE) {
				/* add prop/value to all pending rules */
				for (j = 0; j < state->num_rules; j++) 
					css_rule_add_one(state->rules[j], state->property_buffer, strtrim(state->value_buffer));    
				//printf("Added `%s=%s` to %d rule(s)\n", buf3, buf4, num_rules);
			}
			else if (state->sector != INNER_SPACE) {
				fprintf(stderr, "Unexpected '}', expected value or ';'\n");
				return 1;
			}
			/* reset rules */
			state->num_rules = 0;

			state->sector = OUTER_SPACE;
			continue;
		}

		if (!isspace(c)) {
			if (state->sector == INNER_SPACE) { /* start of property name */
				state->sector = PROPERTY;
				CHANGE_BUFFER(state->property_buffer); /* select "property" buffer for output */ 
			}
			if (state->sector == MIDDLE_SPACE) { /* start of value */
				state->sector = VALUE;
				CHANGE_BUFFER(state->value_buffer); /* select "value" buffer for output */ 
			}
		}

		if (c == '"') { /* push/pop in-string */
			if (state->sector == SUBSTRING) {
				state->sector = state->prev_sector;
				state->prev_sector = 0;
			} else {
				state->prev_sector = state->sector;
				state->sector = SUBSTRING;
			}
			continue;
		}

		if (isalpha(c) || c == '*' || c == '.' || c == '#') { 
			if (state->sector == OUTER_SPACE) { /* start of selector(s) */
				state->sector = SELECTORS;

				CHANGE_BUFFER(state->selector_buffer); /* select "selectors" buffer for output */ 
			}
		}

		/* Buffer overflow */
		if (state->tobuf_space < 2) {
			fprintf(stderr, "No buffer space to store selector/property/value, ABORTING parsing.\n");
			return 1;
		}

		//printf("Saving `%c`, space left: %d\n", c, tobuf_space);

		/* Save character to either buffer */
		*state->tobuf++ = c; 
		*state->tobuf = 0;
		state->tobuf_space--;
	}

	return 0;
}

css_ruleset* css_load_file(const char *filename) {

	css_ruleset *style = NULL;
	css_parser *state;
	FILE *f;

	char buf[1024];
	int n, err;

	f = fopen(filename, "r");
	if (f == NULL) return NULL; /* Failed to open file */

	state = css_parser_create();
	if (state == NULL) { /* Failed to malloc */
		fclose(f);
		return NULL;
	}

	/* Read file */
	while (!feof(f)) {

		n = fread(buf, 1, sizeof(buf), f);

		err = css_parse(state, buf, n);

		if (err) {
			/* Abnormal termination */
			fclose(f);
			css_parser_done(state);
			return NULL;
		}

	}

	/* Normal termination */
	fclose(f);
	style = css_parser_done(state);
	return style;
}

int css_select_into(dom_iter *walker, void *root, css_target *target, void **result, size_t n, int offset, int nest) {
	/* Iterate over all dom objects in root */
	void *iter = root;
	while (iter) {

		css_target *filter;	
		char *cmp;
		int ok;

		/* Update iterator */
		(*walker->pick)(walker, iter);

		/* Cycle all filters until a test fails */
		for (filter = target, ok = 1; (filter && ok); filter = filter->next) {
			ok = 0; /* Each test starts out in 'NOT PASSED' state */
			switch (filter->rela) { /* If it's a relationship test, recurse */
				case ' ':
					filter->rela = '&';
					offset = css_select_into(walker, walker->kids, filter, result, n, offset, 1);
					filter->rela = ' '; 
					return offset;
				break;
				case '>':
					filter->rela = '&';
					offset = css_select_into(walker, walker->kids, filter, result, n, offset, 0);
					filter->rela = '>';
					return offset;
				break;
				case '+':
					filter->rela = '&';
					offset = css_select_into(walker, walker->next, filter, result, n, offset, 0);
					filter->rela = '+';
					return offset;
				break;
				case '&': default:
				if (walker->filter) ok = (*walker->filter)(iter, filter); 
				else {
				    dom_iter* tx1 = iter;
				    const char* tx2 = filter->name;
					cmp = (*walker->test)(tx1, tx2);	
					switch (filter->type) { /* Take the test */
						case '~':
							if (cmp && !strspacecmp(cmp, filter->value)) ok=1;
						break;
						case '|':
							if (cmp && !strstartcmp(cmp, filter->value)) ok=1;
						break;
						case '=': default:
							if (filter->value[0] == '*' 
							|| (cmp && !strcasecmp(cmp, filter->value))) ok=1;
					}
				}
			}
		}
		if (ok) {
			result[offset++] = iter;
			if (offset >= n) break;
		}
		if (!nest) break;
		/* Next object: */
		iter = NULL;
		if (walker->kids)		iter = walker->kids;
		else if (walker->next)	iter = walker->next;
		else if (walker->parent) iter = walker->cousin;
	}
	result[offset] = NULL;
	return offset;
}

void css_assist_apply(dom_iter *walker, void *target, stash_t *node) {
	stash_entry *en;
	int i;
	for (i = 0; i < node->fill; i++) {
		en = node->table + i;
		walker->apply(target, (const char*)en->data, (const char*)&en->data[en->key_size]);
	}
}

void css_cascade(dom_iter *walker, void *dom, css_ruleset *css) {
	css_rule *rule;
	int i, n;
	int max = walker->count(walker, dom);
	/* For each CSS rule... */
	for (rule = css; rule; rule = rule->next) {
		/* ...select all DOM elements */
		void *top[max]; /* passive allocation C99 style */
		n = css_select_into(walker, dom, rule->target, top, max, 0, 1);
		/* And apply to them */
		for (i = 0; i < n; i++) {
			if (walker->merge) walker->merge(top[i], rule);
			else css_assist_apply(walker, top[i], rule->apps);
		}		
	}
}


int css_target_eq(css_target *A, css_target *B) {
	if (A == B) return 1;
	if (strcasecmp(A->name, B->name)) return 0;
	if (A->value[0] != '*' && B->value[0] != '*'
		&& strcasecmp(A->value, B->value)) return 0;
	if ((A->next && !B->next) || (!A->next && B->next)) return 0;
	if (A->next == B->next && B->next == NULL) return 1;
	return css_target_eq(A->next, B->next);
}

css_rule* css_find(css_ruleset *set, css_target *target) {
	css_rule *match;
	for (match = set; match; match = match->next)
		if (css_target_eq(target, match->target)) return match;
	return NULL;
}

/* Micro-dom */
dom_object* dom_create(const char *type, const char *id, const char *class) {
	dom_object *dom = malloc(sizeof(dom_object));
	if (dom) {
		dom->attrs = stash_new();
		if (dom->attrs) {
		
			stash_put(dom->attrs, "type", 5, H_PTR(type), strlen(type)+1);
			stash_put(dom->attrs, "class", 6, H_PTR(class), strlen(class)+1);
			stash_put(dom->attrs, "id", 3, H_PTR(id), strlen(id)+1);

			dom->nextSibling = NULL;
			dom->parentNode = NULL;
			dom->childNode = NULL;

		} else { free(dom); dom = NULL; }
	}
	return dom;
}
int base_count(dom_iter* this, void* ptr) {
	dom_object *dom = ptr;
	return dom_count(dom) - 1;
}
int base_pick(dom_iter* this, void* ptr) {
	dom_object *obj = ptr;
	this->current = ptr;
	if (!this->root) this->root = ptr;
	this->next = (void*)obj->nextSibling;
	this->kids = (void*)obj->childNode;
	this->parent = (void*)obj->parentNode;
	this->prev = NULL;
	this->cousin = (obj->parentNode ? obj->parentNode->nextSibling : NULL);
	return 0;
}
char* base_test(void *ptr, const char *name) {
	dom_object *obj = ptr;
	char *cmp = NULL;
	if (obj->attrs)
	 	cmp = (char*)stash_peek(obj->attrs, name, strlen(name)+1);
	return cmp;
}
int base_apply(void *ptr, const char *name, const char *value) {
	dom_object *obj = ptr;
	if (obj->attrs)
		stash_put(obj->attrs, name, strlen(name)+1, (byte*)value, strlen(value)+1);
	return 0;
}
int base_merge(void *ptr, css_rule *rule) {
	return 0;
}
int base_filter(void *ptr, css_target *filter) {
	printf("Got filter\n");
	return 0;
}

dom_iter dom_object_walker = {
	NULL,
	NULL,
	NULL,
	NULL,
	
	NULL,
	NULL,
	NULL,
	0,
	
	&base_count,
	&base_pick,
	&base_test,
	&base_apply,
	NULL, //&base_merge,
	NULL, //&base_filter,
};



/* Debug functions */
void stash_fprintf(FILE *s, stash_t *node, int edepth) {
	int i,j;
	for (i = 0; i < node->fill; i++) {
		stash_entry *e = node->table + i;
		for (j = 0; j < edepth+1; j++) fprintf(s, "\t");
		fprintf(s, "`%s`: ", e->data);
		fprintf(s, "\"%s\"", &e->data[e->key_size]);
		fprintf(s, " (%d bytes)\n", e->val_size); 
	}
}
void css_fprintf(FILE *s, css_ruleset *style) {
	css_rule *node = style;
	int i = 0;
	fprintf(s, "---------Inspecting Ruleset %p:\n", style);
	for (; node; node = node->next) {
		css_target *filter = node->target; 
		fprintf(s, "(%p)Rule #%d:\n", filter, i++);
		for (; filter; filter = filter->next) {
			fprintf(s, "\tFilter: %c(%c)(%s) %s`\n", filter->rela, filter->type, filter->name, filter->value );
		}
		stash_fprintf(s, node->apps, 1);
	}
}
void dom_fprintf(FILE *s, dom_object *obj) {
	static int depth;
	if (depth == 0) fprintf(s, "----------Inspecting Object %p:\n", obj);
	int i;
	for (i = 0; i < depth; i++) fprintf(s, "\t");
	fprintf(s, "<%s class='%s' id='%s'>\n", 
		stash_peek(obj->attrs, "type",5), 
		stash_peek(obj->attrs, "class",6), 
		stash_peek(obj->attrs, "id",3));
	if (obj->attrs) stash_fprintf(s, obj->attrs, depth);

	depth++;	
	dom_object *node = obj->childNode;
	for ( ; node; node = node->nextSibling)
	{
		dom_fprintf(s, node); 
	}
	depth--;
	for (i = 0; i < depth; i++) fprintf(s, "\t");
	fprintf(s, "</%s>\n", stash_peek(obj->attrs, "type",5));
}

/* String functions */
/* Determine if needle is part of haystack, as per CSS2.1 |= rules */
int strstartcmp(const char *haystack, const char *needle) {
	while (*haystack) {
		if (*haystack == '-' && !*needle) return 0;
		if (*needle++ != *haystack++) return 1;
	}
	return (*needle ? 1 : 0);
}
/* Determine if needle as part of space-separated haystack, as per CSS2.1 ~= rules */ 
int strspacecmp(const char *haystack, const char *needle) {
	const char *reneed = needle;
	while (*haystack) {
		if (*haystack == ' ') reneed = needle;
		if (*haystack++ == *reneed)
			if (*++reneed == 0 && 
				(*haystack == 0 || *haystack == ' ')) return 0;
	}
	return 1;
}
/* Trim whitespace. Cuts original variable (!), returns offset pointer */
char *strtrim(char *str) {
	while (isspace(*str)) str++; /* left trim */
	if (*str) {	/* right trim */
		char *end = str, *mark = str;
		while (*end) if (!isspace(*end++)) mark = end;
		*(mark) = 0;
	}
	return str;
}
