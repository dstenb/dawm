#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bar.h"
#include "client.h"
#include "settings.h"
#include "sysinfo.h"

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

struct ws {
	char name[WS_NAME_SIZE]; /* workspace name */
	LayoutID layout;         /* workspace layout id */
	float mfact;             /* master size factor [0, 1] */
	int nmaster;             /* number of master clients */
};

struct monitor {
	int num;                    /* monitor number, 0... */
	struct bar *bar;            /* bar */
	int mx, my, mw, mh;         /* monitor geometry */
	int wx, wy, ww, wh;         /* window geometry */
	struct client *clients;     /* client list */
	struct client *cstack;      /* client stack */
	struct client *sel;         /* selected client */
	unsigned long selws;        /* selected workspace */
	struct ws ws[N_WORKSPACES]; /* workspace information */
	struct monitor *next;       /* next monitor */
};

void monitor_add_client(struct monitor *, struct client *, Display *, Window);
struct monitor *monitor_append(struct monitor *, struct monitor *);
void monitor_arrange(struct monitor *, Display *);
int monitor_count(struct monitor *);
struct monitor *monitor_create(int, int, int, int, int,
		Display *, Window, int);
void monitor_draw_bar(struct monitor *, Display *);
void monitor_float_selected(struct monitor *, Display *, int);
void monitor_focus(struct monitor *, struct client *, Display *, Window);
struct monitor *monitor_free(struct monitor *, Display *);
void monitor_remove_client(struct monitor *, struct client *);
void monitor_select_client(struct monitor *, struct client *,
		Display *, Window, int);
void monitor_select_next_client(struct monitor *, Display *, Window);
void monitor_select_prev_client(struct monitor *, Display *, Window);
void monitor_selected_to_master(struct monitor *);
void monitor_set_layout(struct monitor *, Display *, int);
void monitor_set_ws(struct monitor *, Display *, Window, unsigned long);
void monitor_show_bar(struct monitor *, Display *, int);
void monitor_toggle_bar(struct monitor *, Display *);
void monitor_unfocus_selected(struct monitor *, Display *, Window);

struct client *find_client_by_trans(struct monitor *, Display *, Window);
struct client *find_client_by_window(struct monitor *, Window);

struct monitor *find_monitor_by_pos(struct monitor *, int, int);
struct monitor *find_monitor_by_ws(struct monitor *, unsigned);

#endif
