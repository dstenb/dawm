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
	"_NET_WM_WINDOW_TYPE"
};

static Atom netatoms[LASTNetAtom];

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
	XChangeProperty(dpy, root, netatom(NetSupported), XA_ATOM, 32,
			PropModeReplace, (unsigned char *) netatoms,
			ARRSIZE(netatoms));
}

void
ewmh_root_client_list_add(Display *dpy, Window root, Window win)
{
	XChangeProperty(dpy, root, netatom(NetClientList), XA_WINDOW, 32,
			PropModeAppend, (unsigned char *) &(win), 1);
}

void
ewmh_root_client_list_clear(Display *dpy, Window root)
{
	XDeleteProperty(dpy, root, netatom(NetClientList));
}

void
ewmh_root_set_active_window(Display *dpy, Window root, Window win)
{
	XChangeProperty(dpy, root, netatom(NetActiveWindow), XA_WINDOW, 32,
			PropModeReplace, (unsigned char *) &(win), 1);
}

void
ewmh_root_set_current_desktop(Display *dpy, Window root, unsigned n)
{
	XChangeProperty(dpy, root, netatom(NetCurrentDesktop), XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &n, 1);
}

void
ewmh_root_set_number_of_desktops(Display *dpy, Window root, unsigned desktops)
{
	XChangeProperty(dpy, root, netatom(NetDesktops), XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &desktops, 1);
}

void
ewmh_root_set_desktop_names(Display *dpy, Window root, unsigned char *names,
		unsigned n)
{
	XChangeProperty(dpy, root, netatom(NetDesktopNames), atom(UTF8String),
			8, PropModeReplace, names, n);
}

int
ewmh_client_get_desktop(Display *dpy, Window win, unsigned long *d)
{
	Atom a;
	int f;
	unsigned long n, r;
	unsigned char *p;

	if (XGetWindowProperty(dpy, win, netatom(NetDesktop), 0, 1L, False,
				XA_CARDINAL, &a, &f, &n, &r, &p) == Success) {
		if (p) {
			*d = *(unsigned long *)p;
			return 1;
		}
	}

	return 0;
}

void
ewmh_client_set_desktop(Display *dpy, Window win, unsigned long d)
{
	XChangeProperty(dpy, win, netatom(NetDesktop), XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &d, 1);
}
