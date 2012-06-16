#ifndef _BAR_H_
#define _BAR_H_

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>

#include "cursors.h"
#include "utils.h"

struct bar {
	/* TODO font */
	Drawable drawable;
	GC gc;
	Window win;

	int topbar;
	int showbar;

	int x, y, w, h;
};

struct bar *bar_create(int, int, int, int, int, int, Display *, Window, int);

void bar_draw(struct bar *, Display *);

#endif
