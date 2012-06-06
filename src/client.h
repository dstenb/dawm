#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>

#include "common.h"
#include "utils.h"

struct client {
	struct rect cur_r; /* current position and size */
	struct rect old_r; /* old position and size */

	int floating; /* non-zero if floating */
	int fullscreen; /* non-zero if fullscreen */

	Window win;

	struct client *next;
};

struct client *client_create(Window, XWindowAttributes *);

#endif
