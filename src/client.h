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

struct client {
	char name[CLIENT_NAME_SIZE];

	/* current position and size */
	struct rect cur_r;
	/* old position and size */
	struct rect old_r;

	/* non-zero if floating */
	int floating;
	/* non-zero if fullscreen */
	int fullscreen;

	/* border size */
	int bsize;
	/* old border size (before the WM took control of it) */
	int old_bsize;

	Window win;

	struct monitor *mon;

	struct client *next;
	struct client *snext;
};

/* create a client */
struct client *client_create(Window, XWindowAttributes *);

/* set focus on the client */
void client_focus(struct client *, Display *, Window);

/* free the client */
void client_free(struct client *);

/* grab buttons */
void client_grab_buttons(struct client *, Display *);

/* returns non-zero if the window is visible */
int client_is_visible(struct client *);

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

/* remove focus from client */
void client_unfocus(struct client *, Display *, Window);

/* unmap the client and revert window settings */
void client_unmap(struct client *, Display *);

/* update the client title */
void client_update_title(struct client *, Display *);

/* initializes the colors that are used by all clients */
void clients_init_colors(struct config *, Display *, int);

#endif
