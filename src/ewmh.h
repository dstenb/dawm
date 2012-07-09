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
	NetDesktopNames,
	NetSupported,
	NetWMName,
	NetWMState,
	NetWMStateFullscreen,
	NetWMWindowType,
	NetWMWindowTypeDialog,
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
void ewmh_root_set_current_desktop(Display *, Window, unsigned long);
void ewmh_root_set_name(Display *, Window, char *);
void ewmh_root_set_number_of_desktops(Display *, Window, unsigned long);
void ewmh_root_set_desktop_names(Display *, Window, unsigned char *, unsigned);

/* application window properties
 * http://standards.freedesktop.org/wm-spec/wm-spec-latest.html#id2577833
 */
int ewmh_client_get_desktop(Display *, Window, unsigned long *);
int ewmh_client_get_state(Display *, Window, Atom *);
int ewmh_client_get_window_types(Display *, Window, Atom **, unsigned *);
void ewmh_client_set_desktop(Display *, Window, unsigned long);

#endif
