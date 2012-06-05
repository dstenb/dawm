#ifndef _WM_H_
#define _WM_H_

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>

#include "utils.h"

struct wm {
	Display *dpy;
	Window root;
	int screen;
	int restart;
	int width, height;
};

struct wm *wm_init(void);

int wm_eventloop(struct wm *);

int wm_destroy(struct wm *);


#endif
