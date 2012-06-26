#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <stdio.h>
#include <stdlib.h>

#include "bar.h"
#include "client.h"
#include "config.h"

#define ISARRANGED(M) ((M)->ws[(M)->selws].layout != FloatingLayout)

#define WS_NAME_SIZE 32

typedef enum {
	TileHorzLayout,
	TileVertLayout,
	MatrixLayout,
	FloatingLayout,
	MaxLayout,
	LASTLayout
} LayoutID;

#define DEFAULT_LAYOUT TileHorzLayout

struct workspace {
	char name[WS_NAME_SIZE]; /* workspace name */
	LayoutID layout;         /* workspace layout id */
	float mfact;             /* master size factor [0, 1] */
	int nmaster;             /* number of master clients */
};

struct monitor {
	int num;                           /* monitor number, 0... */
	struct bar *bar;                   /* bar */
	int mx, my, mw, mh;                /* monitor geometry */
	int wx, wy, ww, wh;                /* window geometry */
	struct client *clients;            /* client list */
	struct client *cstack;             /* client stack */
	struct client *sel;                /* selected client */
	int selws;                         /* selected workspace */
	struct workspace ws[N_WORKSPACES]; /* workspace information */
	struct monitor *next;              /* next monitor */
};

/* add client to a monitor */
void monitor_add_client(struct monitor *, struct client *);

/* append a monitor to another monitor list */
struct monitor *monitor_append(struct monitor *, struct monitor *);

/* arrange the clients on the monitor */
void monitor_arrange(struct monitor *, Display *);

/* creates a monitor */
struct monitor *monitor_create(struct config *, int, int, int, int, int,
		Display *, Window, int);

/* set the floating state for the selected client */
void monitor_float_selected(struct monitor *, Display *, int);

/* focus on the given client. If no client given, the first available
 * client will be focused */
void monitor_focus(struct monitor *, struct client *, Display *, Window);

/* remove the given client from the monitor */
void monitor_remove_client(struct monitor *, struct client *);

/* select the given client, assumes that the client is valid */
void monitor_select_client(struct monitor *, struct client *);

void monitor_select_next_client(struct monitor *, Display *, Window);

void monitor_select_prev_client(struct monitor *, Display *, Window);

void monitor_set_layout(struct monitor *, Display *, int);

/* set the current workspace */
void monitor_set_ws(struct monitor *, Display *, Window, int);

/* show/hide the bar */
void monitor_show_bar(struct monitor *, Display *, int);

/* toggle the bar */
void monitor_toggle_bar(struct monitor *, Display *);

/* removes focus from the currently selected client */
void monitor_unfocus_selected(struct monitor *, Display *, Window);

/* show/hide windows, arrange and focus on first available window */
void monitor_update(struct monitor *, Display *, Window);

/* searches through all the monitors for the client
 * corresponding to the given window */
struct client *find_client_by_window(struct monitor *, Window);

/* returns the monitor that corresponds to the given position */
struct monitor *find_monitor_by_pos(struct monitor *, int, int);

#endif
