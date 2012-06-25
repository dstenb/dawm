#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>

#include "atoms.h"
#include "common.h"
#include "config.h"
#include "utils.h"
#include "xutils.h"

#define CLIENT_NAME_SIZE 128

#define ISVISIBLE(c)            (c->ws == c->mon->selws)

#define WIDTH(c)                ((c)->w + 2 * (c)->bw)
#define HEIGHT(c)               ((c)->h + 2 * (c)->bw)

struct client {
	char name[CLIENT_NAME_SIZE]; /* title */
	int x, y, w, h;              /* current position and size */
	int ox, oy, ow, oh;          /* old position and size */
	int floating;                /* non-zero if floating */
	int fullscreen;              /* non-zero if fullscreen */
	int bw;                      /* border size */
	int obw;                     /* old border size */
	Window win;                  /* window that belongs to the client */
	struct monitor *mon;         /* monitor that the client is on */
	int ws;                      /* workspace that the client is on */
	struct client *next;         /* next client in list */
	struct client *snext;        /* next client in stack */
};

/* create a client */
struct client *client_create(Window, XWindowAttributes *);

/* set focus on the client */
void client_focus(struct client *, Display *, Window);

/* free the client */
void client_free(struct client *);

/* grab buttons */
void client_grab_buttons(struct client *, Display *);

/* kill the client. the removal will be handled in wm_handler_destroynotify */
void client_kill(struct client *, Display *);

void client_map_window(struct client *, Display *);

/* move and resize the client */
void client_move_resize(struct client *, Display *, int, int, int, int);

/* raise the client */
void client_raise(struct client *, Display *);

/* set the client's events to listen for */
void client_select_input(struct client *, Display *);

/* set the border size and color */
void client_set_border(struct client *, Display *, int);

/* set the WM_STATE */
void client_set_state(struct client *, Display *, long);

void client_setup(struct client *, struct config *, struct monitor *,
		Display *, Window, XWindowAttributes *);

void client_show(struct client *, Display *, int);

/* remove focus from client */
void client_unfocus(struct client *, Display *, Window);

/* unmap the client and revert window settings */
void client_unmap(struct client *, Display *);

/* update the client title */
void client_update_title(struct client *, Display *);

#endif
