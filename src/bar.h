#ifndef _BAR_H_
#define _BAR_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#ifdef XFT
#include <X11/Xft/Xft.h>
#endif /* XFT */

#include "colors.h"
#include "cursors.h"
#include "utils.h"
#include "x11.h"

struct bar {
	Window win;     /* bar window */
	bool topbar;    /* top/bottom of the screen */
	bool showbar;   /* show/hide the bar */
	int x, y, w, h; /* bar geometry */
};

struct bar *bar_create(bool, bool, int, int, int);
void bar_draw(struct bar *, const char *str);
void bar_free(struct bar *);

void bars_init(const char *fontstr);
void bars_free(void);

#endif
