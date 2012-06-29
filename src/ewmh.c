#include "ewmh.h"

static void
ewmh_init_desktops(Display *dpy, Window root, unsigned desktops)
{
}

static void
ewmh_init_supported(Display *dpy, Window root)
{
 	Atom atoms[] = {
		atom(NetActiveWindow),
		atom(NetClientList),
		atom(NetSupported),
	};

	XChangeProperty(dpy, root, atom(NetSupported), XA_ATOM, 32,
			PropModeReplace, (unsigned char *) atoms,
			ARRSIZE(atoms));
}

void
ewmh_init(Display *dpy, Window root, unsigned desktops)
{
	ewmh_init_supported(dpy, root);
	ewmh_init_desktops(dpy, root, desktops);
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

