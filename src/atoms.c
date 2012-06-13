#include "atoms.h"

static char *atom_names[LASTAtom] = {
	"WM_CHANGE_STATE",
	"WM_NAME",
	"WM_PROTOCOLS",
	"WM_STATE",
	"WM_TAKE_FOCUS",

	"_NET_ACTIVE_WINDOW",
	"_NET_CLIENT_LIST",
	"_NET_WM_NAME",
	"_NET_WM_WINDOW_TYPE"
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

int has_wm_protocol(Display *dpy, Window win, Atom prot)
{
	int n;
	int found = 0;
	Atom *protocols;

	if (XGetWMProtocols(dpy, win, &protocols, &n)) {
		while (!found && n--)
			found = protocols[n] == prot;
		XFree(protocols);
	}

	return found;
}

void net_client_list_add(Display *dpy, Window root, Window win)
{
	XChangeProperty(dpy, root, atom(NetClientList), XA_WINDOW, 32,
			PropModeAppend, (unsigned char *) &(win), 1);
}

void net_client_list_clear(Display *dpy, Window root)
{
	XDeleteProperty(dpy, root, atom(NetClientList));
}
