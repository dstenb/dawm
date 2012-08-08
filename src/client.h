#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#include "atoms.h"
#include "common.h"
#include "ewmh.h"
#include "monitor.h"
#include "utils.h"
#include "xutils.h"

#define CLIENT_NAME_SIZE 128

#define ISDOCK(c)               (c->wtype & Dock)
#define ISRESIZABLE(c)          (c->floating || !ISARRANGED(c->mon))
#define ISSELECTABLE(c)         (ISVISIBLE(c) && !ISDOCK(c))
#define ISTILED(c)              (!c->floating && ISVISIBLE(c))
#define ISVISIBLE(c)            (c->ws == c->mon->selws_i || c->ws == ALL_WS)

#define WIDTH(c)                ((c)->w + 2 * (c)->bw)
#define HEIGHT(c)               ((c)->h + 2 * (c)->bw)

typedef enum {
	Normal = 1 << 0,
	Dialog = 1 << 1,
	Dock = 1 << 2
} ClientType;

struct client {
	char name[CLIENT_NAME_SIZE]; /* title */
	int x, y, w, h;              /* current position and size */
	int ox, oy, ow, oh;          /* old position and size */
	bool floating;               /* floating state */
	bool fullscreen;             /* fullscreen state */
	bool neverfocus;             /*    */
	int ostate;                  /* old state */
	int bw;                      /* border size */
	int obw;                     /* old border size */
	int wtype;                   /* window type bitmask */
	Window win;                  /* window that belongs to the client */
	struct strut_data *strut;    /* strut data */
	struct monitor *mon;         /* monitor that the client is on */
	unsigned long ws;            /* workspace that the client is on */
	struct client *prev;         /* previous client in list */
	struct client *next;         /* next client in list */
	struct client *snext;        /* next client in stack */
};

struct client *client_create(Window, XWindowAttributes *);
void client_free(struct client *);
void client_kill(struct client *, Display *);
void client_map_window(struct client *, Display *);
void client_move_resize(struct client *, Display *, int, int, int, int);
void client_raise(struct client *, Display *);
void client_set_floating(struct client *, Display *, int);
void client_set_focus(struct client *, Display *, Window, bool);
void client_set_fullscreen(struct client *, Display *, int);
void client_set_state(struct client *, Display *, long);
void client_set_ws(struct client *, Display *, unsigned long);
void client_setup(struct client *, struct monitor *, struct monitor *,
		Display *, Window, struct client *);
void client_show(struct client *, Display *, int);
void client_unmap(struct client *, Display *);
void client_update_size_hints(struct client *, Display *);
void client_update_title(struct client *, Display *);
void client_update_window_type(struct client *, Display *);
void client_update_wm_hints(struct client *, Display *, int);

#endif
