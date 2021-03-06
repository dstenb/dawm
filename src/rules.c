#include "dawm.h"

struct rule *rule_append(struct rule *, struct rule *);
static int rule_applicable(const struct rule *, const char *,
		const char *, const char *);
static void rule_apply(struct rule_settings *, struct client *);
struct rule_match *rule_create_match(const char *, const char *, const char *);
struct rule_settings *rule_create_settings(void);
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
rule_apply(struct rule_settings *s, struct client *c)
{
	if (s->set_ws && VALID_WORKSPACE(s->ws))
		client_set_ws(c, s->ws);

	switch (s->honor_size) {
		case RuleTrue:
			c->shints->honor = true;
			break;
		case RuleFalse:
			c->shints->honor = false;
			break;
		case RuleIgnore:
			break;
	}

	switch (s->floating) {
		case RuleTrue:
			client_set_floating(c, true);
			break;
		case RuleFalse:
			client_set_floating(c, false);
			break;
		case RuleIgnore:
			break;
	}

	switch (s->fullscreen) {
		case RuleTrue:
			client_set_fullscreen(c, true);
			break;
		case RuleFalse:
			client_set_fullscreen(c, false);
			break;
		case RuleIgnore:
			break;
	}
}

struct rule *
rule_create(const char *class, const char *instance, const char *title)
{
	struct rule *rule = xcalloc(1, sizeof(struct rule));

	rule->match = rule_create_match(class, instance, title);
	rule->settings = rule_create_settings();
	rule->next = NULL;

	return rule;
}

struct rule_match *
rule_create_match(const char *c, const char *i, const char *t)
{
	struct rule_match *m = xcalloc(1, sizeof(struct rule_match));

	m->class = c ? xstrdup(c) : NULL;
	m->instance = i ? xstrdup(i) : NULL;
	m->title = t ? xstrdup(t) : NULL;

	return m;
}

struct rule_settings *
rule_create_settings(void)
{
	struct rule_settings *s = xcalloc(1, sizeof(struct rule_settings));

	s->mn = -1;
	s->ws = 0;
	s->set_ws = false;
	s->switch_to_ws = RuleIgnore;
	s->floating = RuleIgnore;
	s->fullscreen = RuleIgnore;
	s->honor_size = RuleIgnore;

	return s;
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
rules_add(struct rule *rule)
{
	_rules = rule_append(_rules, rule);
}

void
rules_apply(struct client *c)
{
	const char *class, *instance;
	struct rule *rule;
	XClassHint hint = { NULL, NULL};

	XGetClassHint(dpy, c->win, &hint);

	class = hint.res_class ? hint.res_class : "";
	instance = hint.res_name ? hint.res_name : "";

	for (rule = _rules ; rule; rule = rule->next) {
		if (rule_applicable(rule, class, instance, c->name))
			rule_apply(rule->settings, c);
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
