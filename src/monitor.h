#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <stdlib.h>
#include <stdio.h>

#include "bar.h"
#include "client.h"
#include "config.h"

struct monitor {
	struct bar *bar;

	int mx, my, mw, mh;
	int wx, wy, ww, wh;

	struct client *clients;
	struct client *cstack;
	struct client *sel;

	struct monitor *next;
};

/* add client to a monitor */
void monitor_add_client(struct monitor *, struct client *);

/* append a monitor to another monitor list */
struct monitor *monitor_append(struct monitor *, struct monitor *);

/* arrange the clients on the monitor */
void monitor_arrange(struct monitor *, Display *);

/* creates a monitor */
struct monitor *monitor_create(struct config *, int, int, int, int,
		Display *, Window, int);

/* focus on the given client. If no client given, the first available
 * client will be focused */
void monitor_focus(struct monitor *, struct client *, Display *, Window);

/* remove the given client from the monitor */
void monitor_remove_client(struct monitor *, struct client *);

/* select the given client, assumes that the client is valid */
void monitor_select_client(struct monitor *, struct client *);

/* searches through all the monitors for the client responding to the window */
struct client *find_client_by_window(struct monitor *, Window);

/* returns the monitor that corresponds to the given position */
struct monitor *find_monitor_by_pos(struct monitor *, int, int);

/* prints a debug message about the given monitor */
void monitor_dbg_print(struct monitor *m, const char *str);

/* removes focus from the currently selected client */
void monitor_unfocus_selected(struct monitor *, Display *, Window);

/* show/hide the bar */
void monitor_show_bar(struct monitor *, Display *, int);

/* toggle the bar */
void monitor_toggle_bar(struct monitor *, Display *);

/* set the floating state for the selected client */
void monitor_float_selected(struct monitor *, Display *, int);

#endif
