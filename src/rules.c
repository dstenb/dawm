#include "rules.h"

struct rule *rule_append(struct rule *, struct rule *);
static int rule_applicable(const struct rule *, const char *, const char *,
		const char *);
static void rule_apply(struct rule *, struct client *);
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
#define MATCHES(r, s) (!(r) || strmatch((r), (s)))

	return MATCHES(rule->match->class, class) &&
		MATCHES(rule->match->instance, instance) &&
		MATCHES(rule->match->title, title);
}

void
rule_apply(struct rule *rule, struct client *c)
{
	(void)c;
	(void)dpy;

	rule_print(rule);
}

struct rule *
rule_create(const char *class, const char *instance, const char *title)
{
	struct rule *rule = xcalloc(1, sizeof(struct rule));

	rule->match = xcalloc(1, sizeof(struct rule_match));
	rule->settings = xcalloc(1, sizeof(struct rule_settings));

	rule->match->class = class ? xstrdup(class) : NULL;
	rule->match->instance = instance ? xstrdup(instance) : NULL;
	rule->match->title = title ? xstrdup(title) : NULL;

	rule->settings->mn = -1;
	rule->settings->ws = 0;
	rule->settings->set_ws = false;
	rule->settings->switch_to_ws = false;
	rule->settings->floating = false;
	rule->settings->fullscreen = false;
	rule->settings->ignore_hints = false;

	rule->next = NULL;

	return rule;
}

struct rule *
rule_free(struct rule *rule)
{
	struct rule *next = rule->next;

	free(rule->match->class);
	free(rule->match->instance);
	free(rule->match->title);
	free(rule->match);
	free(rule->settings);
	free(rule);

	return next;
}

void
rule_print(const struct rule *rule)
{
	DBG("%s(): '%s' '%s' '%s'\n", __func__, rule->match->class,
			rule->match->instance, rule->match->title);
}

void
rules_add(struct rule *rule)
{
	_rules = rule_append(_rules, rule);
}

void
rules_apply(struct client *c)
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
			rule_apply(rule, c);
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
