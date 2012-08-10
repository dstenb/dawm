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
#include "x11.h"

long get_state(Window win);
int send_event(Window, Atom);
int xerror_dummy(Display *, XErrorEvent *);

#endif
