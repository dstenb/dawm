#ifndef _EWMH_H_
#define _EWMH_H_

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include "atoms.h"

typedef enum {
	NetActiveWindow,
	NetClientList,
	NetCurrentDesktop,
	NetDesktop,
	NetDesktops,
	NetSupported,
	NetWMName,
	NetWMWindowType,
	LASTNetAtom
} NetAtomID;

Atom netatom(NetAtomID id);

void ewmh_init(Display *, Window);

/* add a window to the _NET_CLIENT_LIST */
void ewmh_client_list_add(Display *, Window, Window);

/* removes the _NET_CLIENT_LIST property */
void ewmh_client_list_clear(Display *, Window);

/* set _NET_ACTIVE_WINDOW */
void ewmh_set_active_window(Display *, Window, Window);

/*  set _NET_CURRENT_DESKTOP */
void ewmh_set_current_desktop(Display *, Window, unsigned);

/* set _NET_NUMBER_OF_DESKTOPS and _NET_DESKTOP_NAMES */
void ewmh_set_desktops(Display *, Window, unsigned, unsigned);

/* application window properties
 * http://standards.freedesktop.org/wm-spec/wm-spec-latest.html#id2577833
 */
void ewmh_client_set_desktop(Display *, Window, int);

#endif
