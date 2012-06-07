#ifndef _BAR_H_
#define _BAR_H_

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>

struct bar {
	/* TODO font */
	Drawable drawable;
	GC gc;

	int topbar;
	int showbar;
	int height;

	char cmdbuf[1024];
	char timefmt[64];
};

struct bar *bar_init(void);

void bar_show(struct bar *, int);

void bar_update(struct bar *);

#endif
