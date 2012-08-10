#ifndef _COLORS_H_
#define _COLORS_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/cursorfont.h>
#include <X11/Xlib.h>

#include "utils.h"
#include "x11.h"

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

unsigned long color(ColorID);
void colors_init(char * const[LASTColor]);
const char *color_id2str(ColorID);
int color_str2id(const char *);

#endif
