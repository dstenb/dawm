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
	struct client *focused;

	struct monitor *next;
};

struct monitor *monitor_append(struct monitor *, struct monitor *);

struct monitor *monitor_create(struct config *, int, int);

void monitor_add_client(struct monitor *, struct client *);

/* searches through all the monitors for the client responding to the window */
struct client *find_client_by_window(struct monitor *, Window);

#endif
