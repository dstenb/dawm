#ifndef _EWMH_H_
#define _EWMH_H_

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include "atoms.h"
#include "x11.h"

typedef enum {
	NetActiveWindow,
	NetClientList,
	NetCurrentDesktop,
	NetDesktop,
	NetDesktops,
	NetDesktopNames,
	NetSupported,
	NetWMFullscreen,
	NetWMName,
	NetWMState,
	NetWMStrut,
	NetWMStrutPartial,
	NetWMWindowType,
	NetWMWindowTypeDesktop,
	NetWMWindowTypeDialog,
	NetWMWindowTypeDock,
	LASTNetAtom
} NetAtomID;

struct strut_data {
	/* _NET_WM_STRUT and _NET_WM_STRUT_PARTIAL */
	unsigned long left;
	unsigned long right;
	unsigned long top;
	unsigned long bottom;

	/* _NET_WM_STRUT_PARTIAL specific data */
	unsigned long left_start_y, left_end_y;
	unsigned long right_start_y, right_end_y;
	unsigned long top_start_x, top_end_x;
	unsigned long bottom_start_x, bottom_end_x;
};

Atom netatom(NetAtomID id);

/* init atoms and set supported */
void ewmh_init(void);

/* root window properties
 * http://standards.freedesktop.org/wm-spec/wm-spec-latest.html#id2533796
 */
void ewmh_root_client_list_add(Window);
void ewmh_root_client_list_clear(void);
void ewmh_root_set_active_window(Window);
void ewmh_root_set_current_desktop(unsigned long);
void ewmh_root_set_name(char *);
void ewmh_root_set_number_of_desktops(unsigned long);
void ewmh_root_set_desktop_names(unsigned char *, unsigned);

/* application window properties
 * http://standards.freedesktop.org/wm-spec/wm-spec-latest.html#id2577833
 */
int ewmh_client_get_desktop(Window, unsigned long *);
int ewmh_client_get_state(Window, Atom *);
int ewmh_client_get_strut(Window, struct strut_data *);
int ewmh_client_get_strut_partial(Window, struct strut_data *);
int ewmh_client_get_window_types(Window, Atom **, unsigned *);
void ewmh_client_set_desktop(Window, unsigned long);
void ewmh_client_set_state(Window, Atom);

#endif
