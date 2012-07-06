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

/* init atoms and set supported */
void ewmh_init(Display *, Window);

/* root window properties
 * http://standards.freedesktop.org/wm-spec/wm-spec-latest.html#id2533796
 */
void ewmh_root_client_list_add(Display *, Window, Window);
void ewmh_root_client_list_clear(Display *, Window);
void ewmh_root_set_active_window(Display *, Window, Window);
void ewmh_root_set_current_desktop(Display *, Window, unsigned);
void ewmh_root_set_desktops(Display *, Window, unsigned, unsigned);

/* application window properties
 * http://standards.freedesktop.org/wm-spec/wm-spec-latest.html#id2577833
 */
void ewmh_client_set_desktop(Display *, Window, int);

#endif
