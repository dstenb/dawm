#ifndef _ATOMS_H_
#define _ATOMS_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "utils.h"
#include "x11.h"

typedef enum {
	WMChangeState,
	WMDelete,
	WMName,
	WMProtocols,
	WMState,
	WMTakeFocus,
	UTF8String,
	LASTAtom
} AtomID;

Atom atom(AtomID);
void atoms_init(void);

void atom_append_window(Window, Atom, Window);
void atom_delete(Window, Atom);
int atom_get_atom(Window, Atom, Atom *);
int atom_get_atoms(Window, Atom, Atom **, unsigned *);
int atom_get_cardinal(Window, Atom, unsigned long *);
int atom_get_cardinals(Window, Atom, unsigned long **, unsigned *);
int atom_get_string(Window, Atom, char *, unsigned int);
void atom_set_atom(Window, Atom, Atom);
void atom_set_atoms(Window, Atom, Atom *, unsigned);
void atom_set_cardinal(Window, Atom, unsigned long);
void atom_set_string(Window, Atom, char *);
void atom_set_utf8array(Window, Atom, unsigned char *, unsigned);
void atom_set_window(Window, Atom, Window);

#endif
