#ifndef _ATOMS_H_
#define _ATOMS_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#include "utils.h"

typedef enum {
	WMNameAtom,
	WMStateAtom,
	WMTakeFocusAtom,
	NetActiveWindowAtom,
	LASTAtom
} AtomID;

/* get the atom with the given ID */
Atom atom(AtomID);

/* initializes all atoms */
void atoms_init(Display *);

#endif
