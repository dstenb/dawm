#ifndef _COLORS_H_
#define _COLORS_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/cursorfont.h>
#include <X11/Xlib.h>

#include "utils.h"

typedef enum {
	BarBorder,
	BarNormFG,
	BarNormBG,
	BarSelFG,
	BarSelBG,
	WinNormBorder,
	WinSelBorder,
	LASTColor,
	InvalidColor = -1
} ColorID;

/* get cursor with the given id */
unsigned long color(ColorID);

/* initializes the cursors */
void colors_init(char *[LASTColor], Display *, int);

const char *color_id2str(ColorID);

int color_str2id(const char *);

#endif
