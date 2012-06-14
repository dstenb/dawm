#ifndef _WM_H_
#define _WM_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "atoms.h"
#include "config.h"
#include "cursors.h"
#include "keys.h"
#include "monitor.h"
#include "utils.h"

typedef enum {
	NoMotion,
	ResizeMotion,
	MovementMotion
} MotionType;

struct wm {
	Display *dpy;
	Window root;

	int screen;
	int width, height;

	struct monitor *mons;
	struct monitor *selmon;

	struct key *keys;
	struct config *cfg;
	const char *cmd;

	struct {
		MotionType type;
		XButtonEvent start;
		XWindowAttributes attr;
	} motion;
};

struct wm *wm_init(struct config *, const char *);

int wm_eventloop(struct wm *);

int wm_destroy(struct wm *);

#endif
