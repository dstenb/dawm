#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <stdlib.h>
#include <stdio.h>

#include "bar.h"
#include "client.h"
#include "config.h"

struct monitor {
	struct bar *bar;

	int width;
	int height;

	struct client *clients;
	struct client *cstack;
	struct client *sel;

	struct monitor *next;
};

/* add client to a monitor */
void monitor_add_client(struct monitor *, struct client *);

/* append a monitor to another monitor list */
struct monitor *monitor_append(struct monitor *, struct monitor *);

void monitor_arrange(struct monitor *);

/* creates a monitor */
struct monitor *monitor_create(struct config *, int, int);

void monitor_focus(struct monitor *, struct client *);

void monitor_remove_client(struct monitor *, struct client *);

/* select the given client, assumes that the client is valid */
void monitor_select_client(struct monitor *, struct client *);

/* searches through all the monitors for the client responding to the window */
struct client *find_client_by_window(struct monitor *, Window);

#endif
