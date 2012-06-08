#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>

#include "common.h"
#include "config.h"
#include "utils.h"
#include "xutils.h"

#define CLIENT_NAME_SIZE 128

struct client {
	char name[CLIENT_NAME_SIZE];

	struct rect cur_r; /* current position and size */
	struct rect old_r; /* old position and size */

	int floating; /* non-zero if floating */
	int fullscreen; /* non-zero if fullscreen */

	int bsize; /* border size */
	int old_bsize;

	Window win;

	struct monitor *mon;

	struct client *next;
	struct client *snext;
};

/* client_apply_rules(struct client *, feawfeaw) */

struct client *client_create(Window, XWindowAttributes *);

void client_focus(struct client *, Display *, Window);

void client_free(struct client *);

void client_grab_buttons(struct client *, Display *);

int client_is_visible(struct client *);

void client_raise(struct client *, Display *);

void client_select_input(struct client *, Display *);

void client_set_border(struct client *, Display *, int);

void client_set_name(struct client *, const char *);

void client_set_state(struct client *, Display *, long);

void client_unfocus(struct client *, Display *, Window);

void client_unmap(struct client *, Display *);

void client_update_title(struct client *, Display *);

/* initializes the colors that are used by all clients */
void clients_init_colors(struct config *, Display *, int);

#endif
