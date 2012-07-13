#ifndef _BAR_H_
#define _BAR_H_

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>

#include "colors.h"
#include "cursors.h"
#include "utils.h"

struct bar {
	Window win;     /* bar window */
	int topbar;     /* top/bottom of the screen */
	int showbar;    /* show/hide the bar */
	int x, y, w, h; /* bar geometry */
};

struct bar *bar_create(int, int, int, int, int, Display *, Window, int);
void bar_draw(struct bar *, Display *, const char *str);
void bar_free(struct bar *, Display *);

void bars_init(Display *, Window, int, const char *fontstr);
void bars_free(Display *);

#endif
