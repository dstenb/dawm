#include "ewmh.h"

static char *atom_names[LASTNetAtom] = {
	"_NET_ACTIVE_WINDOW",
	"_NET_CLIENT_LIST",
	"_NET_CURRENT_DESKTOP",
	"_NET_WM_DESKTOP",
	"_NET_NUMBER_OF_DESKTOPS",
	"_NET_DESKTOP_NAMES",
	"_NET_SUPPORTED",
	"_NET_WM_NAME",
	"_NET_WM_STATE",
	"_NET_WM_STATE_FULLSCREEN",
	"_NET_WM_WINDOW_TYPE",
	"_NET_WM_WINDOW_TYPE_DIALOG"
};

static Atom netatoms[LASTNetAtom];
static const unsigned n_netatoms = ARRSIZE(netatoms);

Atom
netatom(NetAtomID id)
{
	assert(id < LASTNetAtom);
	return netatoms[id];
}

void
ewmh_init(Display *dpy, Window root)
{
	error("%s\n", __func__);

	/* init atoms */
	XInternAtoms(dpy, atom_names, ARRSIZE(atom_names), 0, netatoms);

	/* set supported atoms */
	atom_set_atoms(dpy, root, netatom(NetSupported), netatoms, n_netatoms);
}

void
ewmh_root_client_list_add(Display *dpy, Window root, Window win)
{
	atom_append_window(dpy, root, netatom(NetClientList), win);
}

void
ewmh_root_client_list_clear(Display *dpy, Window root)
{
	atom_delete(dpy, root, netatom(NetClientList));
}

void
ewmh_root_set_active_window(Display *dpy, Window root, Window win)
{
	atom_set_window(dpy, root, netatom(NetActiveWindow), win);
}

void
ewmh_root_set_current_desktop(Display *dpy, Window root, unsigned long n)
{
	atom_set_cardinal(dpy, root, netatom(NetCurrentDesktop), n);
}

void
ewmh_root_set_name(Display *dpy, Window root, char *name)
{
	atom_set_string(dpy, root, netatom(NetWMName), name);
}

void
ewmh_root_set_number_of_desktops(Display *dpy, Window root, unsigned long n)
{
	atom_set_cardinal(dpy, root, netatom(NetDesktops), n);
}

void
ewmh_root_set_desktop_names(Display *dpy, Window root, unsigned char *names,
		unsigned n)
{
	atom_set_utf8array(dpy, root, netatom(NetDesktopNames), names, n);
}

int
ewmh_client_get_desktop(Display *dpy, Window win, unsigned long *d)
{
	return atom_get_cardinal(dpy, win, netatom(NetDesktop), d);
}

int
ewmh_client_get_state(Display *dpy, Window win, Atom *state)
{
	return atom_get_atom(dpy, win, netatom(NetWMState), state);
}

int
ewmh_client_get_window_types(Display *dpy, Window win, Atom **t, unsigned *n)
{
	return atom_get_atoms(dpy, win, netatom(NetWMWindowType), t, n);
}

void
ewmh_client_set_desktop(Display *dpy, Window win, unsigned long d)
{
	atom_set_cardinal(dpy, win, netatom(NetDesktop), d);
}
