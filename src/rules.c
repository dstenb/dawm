#include "rules.h"

int rule_applicable(struct rule *, const char *, const char *, const char *);
struct rule *rule_free(struct rule *);

int
rule_applicable(struct rule *rule, const char *class, const char *instance,
		const char *title)
{
	/* TODO */
	return 0;
}

void
rule_apply_all(struct rule *rule, struct client *c)
{
	/* TODO */
}

struct rule *
rule_append(struct rule *rlist, struct rule *new)
{
	struct rule *trav;

	if (rlist) {
		for (trav = rlist; trav->next; trav = trav->next) ;
		trav->next = new;
		return rlist;
	} else {
		return new;
	}
}

struct rule *
rule_create(const char *class, const char *instance, const char *title)
{
	struct rule *rule = xcalloc(1, sizeof(struct rule));

	rule->class = class ? xstrdup(class) : NULL;
	rule->instance = instance ? xstrdup(instance) : NULL;
	rule->title = title ? xstrdup(title) : NULL;

	rule->mn = -1;
	rule->ws = 0;
	rule->set_ws = 0;
	rule->switch_to_ws = 0;
	rule->floating = 0;
	rule->fullscreen = 0;
	rule->ignore_hints = 0;
	rule->next = NULL;

	return rule;
}

struct rule *
rule_free(struct rule *rule)
{
	struct rule *next = rule->next;

	free(rule->class);
	free(rule->instance);
	free(rule->title);
	free(rule);

	return next;
}

struct rule *
rule_free_all(struct rule *rule)
{
	for ( ; rule; rule = rule_free(rule));

	return NULL;
}
