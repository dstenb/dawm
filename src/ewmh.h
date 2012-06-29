#ifndef _EWMH_H_
#define _EWMH_H_

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include "atoms.h"

void ewmh_init(Display *, Window);

/* TODO */
/*  set _NET_CURRENT_DESKTOP */
void ewmh_set_current_desktop(unsigned);

/* TODO */
/* set _NET_ACTIVE_WINDOW */
void ewmh_set_active_window(Display *, Window);

/* add a window to the _NET_CLIENT_LIST */
void ewmh_client_list_add(Display *, Window, Window);

/* removes the _NET_CLIENT_LIST property */
void ewmh_client_list_clear(Display *, Window);

#endif
