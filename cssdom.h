#ifndef __CSSDOM_H_
#define __CSSDOM_H_

#include <stdio.h>	/* For FILE*, fopen, etc */

#include "leaves/leaves.h"
#include "hash/stash.h"

typedef struct css_rule css_ruleset;
typedef struct css_rule css_rule;
typedef struct css_target css_target;

typedef struct css_parser css_parser;

#define SELECTOR_BUFFER 128
#define PROPERTY_BUFFER 128
#define VALUE_BUFFER 256
struct css_parser {

	char selector_buffer[SELECTOR_BUFFER];
	char property_buffer[PROPERTY_BUFFER];
	char    value_buffer[VALUE_BUFFER];

	char *tobuf; /* pointer to one of the above buffers */
	int tobuf_space; /* space left in said buffer */

	css_target *selectors[1024];
	int num_selectors;

    css_rule *rules[1024];
    int num_rules;

    int i, j;

	int sector;
	int prev_sector;

	css_ruleset *style; /* Resulting ruleset */
};

struct css_rule {
	css_target *target;
	stash_t *apps;
	css_rule *next;
};

struct css_target {
	char name[80];
	char value[80];
	char type;
	char rela;
	css_target *next;
};

EXT_LL_ADD_NEXT(css_target, css_target, next);
EXT_LL_ADD_NEXT(css_rule, css_rule, next);
EXT_LL_FREE(css_target, css_target);
EXT_LL_FREE(css, css_ruleset);

extern css_parser* css_parser_create(void);
extern css_ruleset* css_parser_done(css_parser *state);
extern int css_parse(css_parser *state, const char *data, int len);
extern css_ruleset* css_load_file(const char *filename);

extern css_rule* css_rule_create(css_target *target, const char *k, const char *v);
extern css_target* css_target_create(css_target *prev, const char *name, const char *value, const char type, const char rela);

extern css_target* css_target_parse(const char *str);
extern int css_target_eq(css_target *A, css_target *B);

extern css_rule* css_find(css_ruleset *set, css_target *target);

typedef struct dom_iter dom_iter;
struct dom_iter {
	void* root;
	void* next;
	void* prev;
	void* kids;

	void* parent;
	void* cousin;
	void* current;	
	int  cursor;

	int	(*count)(dom_iter *this, void *node);
	int	(*pick)(dom_iter *this, void *node);
	char*(*test)(void *node, const char *name);
	int (*apply)(void *node, const char *name, const char *value);
	int (*merge)(void *node, css_rule *rule);
	int (*filter)(void *node, css_target *filter);
};
#define css_select(WALKER, ROOT, SELECTOR, RESULT, NUM) dom_select_into(WALKER, ROOT, SELECTOR, RESULT, NUM, 0, 1)
extern int css_select_into(dom_iter *walker, void *root, css_target *target, void **result, size_t n, int offset, int nest);
extern void css_cascade(dom_iter *walker, void *dom, css_ruleset *css);

extern char *strtrim(char *str);
extern int strstartcmp(const char *haystack, const char *needle);
extern int strspacecmp(const char *haystack, const char *needle);


typedef struct dom_object dom_object;
struct dom_object {
	stash_t *attrs;

	dom_object *parentNode;
	dom_object *childNode;
	dom_object *nextSibling;	
};
extern dom_object* dom_create(const char *type, const char *id, const char *class);
EXT_LLT_PACKAGE(dom, dom_object, childNode, nextSibling, parentNode);

extern dom_iter dom_object_walker;
#define dom_select(ROOT, SELECTOR, RESULT, NUM) dom_select_into(&dom_object_walker, ROOT, SELECTOR, RESULT, NUM, 0, 1)
#define dom_cascade(ROOT, RULES) css_cascade(&dom_object_walker, ROOT, RULES)

extern void css_fprintf(FILE *s, css_ruleset *style);
extern void dom_fprintf(FILE *s, dom_object *obj);

#endif