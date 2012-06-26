#ifndef _WM_H_
#define _WM_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "atoms.h"
#include "colors.h"
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

struct motion {
	MotionType type;        /* motion type */
	XButtonEvent start;     /* event when the motion was started */
	XWindowAttributes attr; /* window attr. when the motion was started */
};

struct wm {
	Display *dpy;           /* main Display struct */
	Window root;            /* root window */
	int screen;             /* screen number */
	int width, height;      /* total width and height (all monitors) */
	struct monitor *mons;   /* monitor list */
	struct monitor *selmon; /* selected monitor */
	struct key *keys;       /* key bindings */
	struct config *cfg;     /* configuration settings */
	const char *cmd;        /* command used to launch the wm*/
	struct motion motion;   /* mouse motion info */
};

struct wm *init(struct config *, const char *);

int eventloop(struct wm *);

int destroy(struct wm *);

#endif
