#ifndef _ATOMS_H_
#define _ATOMS_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include "utils.h"

typedef enum {
	WMChangeState,
	WMName,
	WMProtocols,
	WMState,
	WMTakeFocus,
	NetActiveWindow,
	NetClientList,
	NetSupported,
	NetWMName,
	NetWMWindowType,
	LASTAtom
} AtomID;

/* get the atom with the given ID */
Atom atom(AtomID);

/* initializes all atoms */
void atoms_init(Display *);

int has_wm_protocol(Display *, Window, Atom);

/* add a window to the NetClientList property */
void net_client_list_add(Display *, Window, Window);

/* delete the NetClientList property */
void net_client_list_clear(Display *, Window);

/* set supported NetWM atoms */
void net_set_supported(Display *, Window);

#endif
