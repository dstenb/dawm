#include "rules.h"

struct rule *rule_append(struct rule *, struct rule *);
static int rule_applicable(const struct rule *, const char *, const char *,
		const char *);
static void rule_apply(struct rule *, struct client *, Display *);
static struct rule *rule_free(struct rule *);
static void rule_print(const struct rule *);

static struct rule *_rules = NULL;

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

int
rule_applicable(const struct rule *rule, const char *class,
		const char *instance, const char *title)
{
	return ((!rule->class || strmatch(rule->class, class))
		&& (!rule->instance || strmatch(rule->instance, instance))
		&& (!rule->title || strmatch(rule->title, title)));
}

void
rule_apply(struct rule *rule, struct client *c, Display *dpy)
{
	(void)c;
	(void)dpy;

	rule_print(rule);
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

void
rule_print(const struct rule *rule)
{
	DBG("%s(): '%s' '%s' '%s'\n", __func__, rule->class,
			rule->instance, rule->title);
}

void
rules_add(struct rule *rule)
{
	_rules = rule_append(_rules, rule);
}

void
rules_apply(struct client *c, Display *dpy)
{
	const char *class;
	const char *instance;
	struct rule *rule;
	XClassHint hint = { NULL, NULL};

	XGetClassHint(dpy, c->win, &hint);

	class = hint.res_class ? hint.res_class : "";
	instance = hint.res_name ? hint.res_name : "";

	DBG("%s(): '%s', '%s', '%s'\n", __func__, class, instance, c->name);

	for (rule = _rules ; rule; rule = rule->next) {
		if (rule_applicable(rule, class, instance, c->name))
			rule_apply(rule, c, dpy);
	}

	if(hint.res_class)
		XFree(hint.res_class);
	if(hint.res_name)
		XFree(hint.res_name);
}

void
rules_free()
{
	for ( ; _rules; _rules = rule_free(_rules));
}
