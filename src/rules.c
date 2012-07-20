#include "rules.h"

int regex_matches(const char *, const char *);

int rule_applicable(struct rule *, const char *, const char *, const char *);
struct rule *rule_free(struct rule *);

/** POSIX regex wrapper that returns non-zero if the given string matches to
 * the given regex. The function uses the POSIX Extended syntax. */
int
regex_matches(const char *regex, const char *str)
{
	regex_t preg;
	int pregret;
	int ret = 0;
	char buf[256];

	if ((pregret = regcomp(&preg, regex, REG_EXTENDED)) == 0) {
		if (regexec(&preg, str, 0, NULL, 0) == 0)
			ret = 1;
	} else {
		regerror(pregret, &preg, buf, sizeof(buf));
		error("%s(\"%s\", \"%s\"): %s\n", __func__, regex, str, buf);
	}

	regfree(&preg);

	return ret;
}

int
rule_applicable(struct rule *rule, const char *class, const char *instance,
		const char *title)
{
	return ((!rule->class || regex_matches(rule->class, class))
		&& (!rule->instance || regex_matches(rule->instance, instance))
		&& (!rule->title || regex_matches(rule->title, title)));
}

void
print_rule(const struct rule *rule)
{
	printf("class: '%s'\n", rule->class);
	printf("instance: '%s'\n", rule->class);
	printf("title: '%s'\n", rule->title);

}

void
rule_apply_all(struct rule *rule, struct client *c, Display *dpy)
{
	const char *class;
	const char *instance;
	const char *title;
	XClassHint hint = { NULL, NULL};

	XGetClassHint(dpy, c->win, &hint);

	class = hint.res_class ? hint.res_class : "";
	instance = hint.res_name ? hint.res_name : "";
	title = c->name;

	for ( ; rule; rule = rule->next) {
		print_rule(rule);

		if (rule_applicable(rule, class, instance, title)) {
			printf(":-) !\n");
		} else {
			printf(":-( !\n");
		}
	}
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
