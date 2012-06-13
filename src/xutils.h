#ifndef _XUTILS_H_
#define _XUTILS_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "atoms.h"
#include "utils.h"

unsigned long get_color(Display *, int, const char *);

int get_text_prop(Display *, Window, Atom, char *, unsigned int);

int send_event(Display *, Window, Atom);

void set_text_prop(Display *, Window, Atom, char *);

#endif
