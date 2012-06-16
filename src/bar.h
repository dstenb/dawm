#ifndef _BAR_H_
#define _BAR_H_

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>

#include "colors.h"
#include "cursors.h"
#include "utils.h"

struct bar {
	/* TODO font */

	Window win;

	int topbar;
	int showbar;

	int x, y, w, h;
};

/* create a bar */
struct bar *bar_create(int, int, int, int, int, int, Display *, Window, int);

/* draw the bar */
void bar_draw(struct bar *, Display *);

/* initialize the draw context used by all bars */
void bars_init_dc(Display *, Window, int, int);

#endif
