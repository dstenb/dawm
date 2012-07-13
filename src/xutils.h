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

long get_state(Display *, Window win);
int send_event(Display *, Window, Atom);

#endif
