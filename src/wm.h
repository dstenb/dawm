#ifndef _WM_H_
#define _WM_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "keys.h"
#include "utils.h"

struct wm {
	Display *dpy;
	Window root;
	int screen;
	int width, height;

	struct key *keys;
	const char *cmd;
};

struct wm *wm_init(const char *);

int wm_eventloop(struct wm *);

int wm_destroy(struct wm *);


#endif
