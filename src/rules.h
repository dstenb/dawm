#ifndef _RULES_H_
#define _RULES_H_

#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#include "client.h"
#include "utils.h"

struct rule {
	char *class;       /* class name regex */
	char *instance;    /* instance name regex */
	char *title;       /* client title regex */

	int mn;            /* monitor number, -1 to avoid this rule */
	unsigned long ws;  /* workspace number */
	int set_ws;        /* set the workspace if set_ws is non-zero*/
	int switch_to_ws;  /* switch to the clients' workspace */
	int floating;      /* float the client */
	int fullscreen;    /* put the client in fullscreen mode */
	int ignore_hints;  /* ignore size hints */
	struct rule *next; /* next rule in rule list */
};

void rule_apply_all(struct rule *, struct client *);
struct rule *rule_append(struct rule *, struct rule *);
struct rule *rule_create(const char *, const char *, const char *);
struct rule *rule_free_all(struct rule *);

#endif
