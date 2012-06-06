#ifndef _BAR_H_
#define _BAR_H_

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>

enum bar_placement {
	BAR_TOP,
	BAR_BOTTOM
};

struct bar {
	/* TODO font */
	Drawable drawable;
	GC gc;
	int visible;
	int height;
	char cmdbuf[1024];
	char timefmt[64];
	int placement;
};

struct bar *bar_init(void);

void bar_show(struct bar *, int);

void bar_update(struct bar *);

#endif
