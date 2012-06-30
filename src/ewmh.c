#include "ewmh.h"

static void
ewmh_init_supported(Display *dpy, Window root)
{
 	Atom atoms[] = {
		atom(NetActiveWindow),
		atom(NetClientList),
		atom(NetCurrentDesktop),
		atom(NetDesktops),
		atom(NetSupported),
	};

	XChangeProperty(dpy, root, atom(NetSupported), XA_ATOM, 32,
			PropModeReplace, (unsigned char *) atoms,
			ARRSIZE(atoms));
}

void
ewmh_init(Display *dpy, Window root)
{
	ewmh_init_supported(dpy, root);
}

void
ewmh_client_list_add(Display *dpy, Window root, Window win)
{
	XChangeProperty(dpy, root, atom(NetClientList), XA_WINDOW, 32,
			PropModeAppend, (unsigned char *) &(win), 1);
}

void
ewmh_client_list_clear(Display *dpy, Window root)
{
	XDeleteProperty(dpy, root, atom(NetClientList));
}

void
ewmh_set_active_window(Display *dpy, Window root, Window win)
{
	XChangeProperty(dpy, root, atom(NetActiveWindow), XA_WINDOW, 32,
			PropModeReplace, (unsigned char *) &(win), 1);
}

void
ewmh_set_current_desktop(Display *dpy, Window root, unsigned n)
{
	XChangeProperty(dpy, root, atom(NetCurrentDesktop), XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &n, 1);
}

void
ewmh_set_desktops(Display *dpy, Window root, unsigned m, unsigned d)
{
	unsigned desktops = m * d;

	XChangeProperty(dpy, root, atom(NetDesktops), XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &desktops, 1);

	/* TODO: set desktop names (monitor i:desktop d maybe?) */
	/* _NET_DESKTOP_NAMES */
}
