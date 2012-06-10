#ifndef _ATOMS_H_
#define _ATOMS_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#include "utils.h"

typedef enum {
	WMChangeStateAtom,
	WMNameAtom,
	WMProtocolsAtom,
	WMStateAtom,
	WMTakeFocusAtom,
	NetActiveWindowAtom,
	LASTAtom
} AtomID;

/* get the atom with the given ID */
Atom atom(AtomID);

/* initializes all atoms */
void atoms_init(Display *);

int has_wm_protocol(Display *, Window, Atom);

#endif
