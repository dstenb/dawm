#include "atoms.h"

static char *atom_names[LASTAtom] = {
	"WM_NAME",
	"WM_STATE",
	"WM_TAKE_FOCUS",

	"_NET_ACTIVE_WINDOW"
};

static Atom atoms[LASTAtom];

Atom atom(AtomID id)
{
	assert(id < LASTAtom);
	return atoms[id];
}

void atoms_init(Display *dpy)
{
	error("%s\n", __func__);
	XInternAtoms(dpy, atom_names, ARRSIZE(atom_names), 0, atoms);
}
