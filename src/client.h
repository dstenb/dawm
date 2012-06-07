#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>

#include "common.h"
#include "utils.h"
#include "xutils.h"

#define CLIENT_NAME_SIZE 128

struct client {
	char name[CLIENT_NAME_SIZE];

	struct rect cur_r; /* current position and size */
	struct rect old_r; /* old position and size */

	int floating; /* non-zero if floating */
	int fullscreen; /* non-zero if fullscreen */

	Window win;

	struct monitor *mon;

	struct client *next;
};

struct client *client_create(Window, XWindowAttributes *);

void client_set_name(struct client *, const char *);

void client_update_title(struct client *, Display *);

#endif
