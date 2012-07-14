#ifndef _ATOMS_H_
#define _ATOMS_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "utils.h"

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
void atoms_init(Display *);

void atom_append_window(Display *, Window, Atom, Window);
void atom_delete(Display *, Window, Atom);
int atom_get_atom(Display *, Window, Atom, Atom *);
int atom_get_atoms(Display *, Window, Atom, Atom **, unsigned *);
int atom_get_cardinal(Display *, Window, Atom, unsigned long *);
int atom_get_string(Display *, Window, Atom, char *, unsigned int);
void atom_set_atom(Display *, Window, Atom, Atom);
void atom_set_atoms(Display *, Window, Atom, Atom *, unsigned);
void atom_set_cardinal(Display *, Window, Atom, unsigned long);
void atom_set_string(Display *, Window, Atom, char *);
void atom_set_utf8array(Display *, Window, Atom, unsigned char *, unsigned);
void atom_set_window(Display *, Window, Atom, Window);

#endif
