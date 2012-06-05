#ifndef _BAR_H_
#define _BAR_H_

#include <stdlib.h>
#include <stdio.h>

enum bar_placement {
	BAR_TOP,
	BAR_BOTTOM
};

struct bar {
	/* TODO font */
	int visible;
	int height;
	char cmdbuf[1024];
	char timefmt[64];
	int placement;
};

struct bar *bar_init(void);

struct bar_show(struct bar *, int);

struct bar_update(struct bar *);

#endif
