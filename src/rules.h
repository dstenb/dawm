#ifndef _RULES_H_
#define _RULES_H_

#include <stdio.h>
#include <stdlib.h>

#include "client.h"
#include "utils.h"
#include "x11.h"

struct client;

struct rule_match {
	char *class;       /* class name regex */
	char *instance;    /* instance name regex */
	char *title;       /* client title regex */
};

struct rule_settings {
	int mn;            /* monitor number, -1 to avoid this rule */
	unsigned long ws;  /* workspace number */
	bool set_ws;        /* set the workspace if set_ws is non-zero*/
	bool switch_to_ws;  /* switch to the clients' workspace */
	bool floating;      /* float the client */
	bool fullscreen;    /* put the client in fullscreen mode */
	bool ignore_hints;  /* ignore size hints */
};

struct rule {
	struct rule_match *match;
	struct rule_settings *settings;
	struct rule *next;
};

struct rule *rule_create(const char *, const char *, const char *);

void rules_add(struct rule *);
void rules_apply(struct client *);
void rules_free(void);

#endif
