#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bar.h"
#include "client.h"
#include "layouts.h"
#include "settings.h"
#include "sysinfo.h"
#include "x11.h"

#define ISARRANGED(M) (true)

#define WS_NAME_SIZE 32

#define DEFAULT_LAYOUT TileHorzLayout

struct ws {
	char name[WS_NAME_SIZE]; /* workspace name */
	struct layout *layout;   /* workspace layout */
};

struct monitor {
	int num;                    /* monitor number, 0... */
	struct bar *bar;            /* bar */
	int mx, my, mw, mh;         /* monitor geometry */
	int wx, wy, ww, wh;         /* window geometry */
	struct client *clients;     /* client list */
	struct client *cstack;      /* client stack */
	struct client *sel;         /* selected client */
	struct ws *selws;           /* selected workspace */
	unsigned long selws_i;      /* selected workspace index */
	unsigned long prevws_i;     /* previous workspace index */
	struct ws ws[N_WORKSPACES]; /* workspace information */
	struct monitor *next;       /* next monitor */
	char str[1024];
};

#define monitor_toggle_bar(M) monitor_show_bar(M, !M->bar->showbar);

void monitor_add_client(struct monitor *, struct client *);
struct monitor *monitor_append(struct monitor *, struct monitor *);
void monitor_arrange(struct monitor *);
int monitor_count(struct monitor *);
struct monitor *monitor_create(int, int, int, int, int);
void monitor_draw_bar(struct monitor *);
void monitor_float_selected(struct monitor *, bool);
void monitor_focus(struct monitor *, struct client *);
struct monitor *monitor_free(struct monitor *);
void monitor_remove_client(struct monitor *, struct client *);
void monitor_select_client(struct monitor *, struct client *, bool);
void monitor_select_next_client(struct monitor *);
void monitor_select_prev_client(struct monitor *);
void monitor_selected_to_master(struct monitor *);
void monitor_set_layout(struct monitor *, int);
void monitor_set_ws(struct monitor *, unsigned long);
void monitor_show_bar(struct monitor *, bool);
void monitor_swap_next_client(struct monitor *);
void monitor_swap_prev_client(struct monitor *);
void monitor_unfocus_selected(struct monitor *);

struct client *find_client_by_trans(struct monitor *, Window);
struct client *find_client_by_window(struct monitor *, Window);

struct monitor *find_monitor_by_num(struct monitor *, int);
struct monitor *find_monitor_by_pos(struct monitor *, int, int);
struct monitor *find_monitor_by_ws(struct monitor *, unsigned);

#endif
