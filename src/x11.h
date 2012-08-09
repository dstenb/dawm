#ifndef _X11_H_
#define _X11_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#include "utils.h"

extern Display *dpy;
extern Window root;
extern int screen;
extern int screen_w;
extern int screen_h;

void x11_init(void);
void x11_destroy(void);

#endif
