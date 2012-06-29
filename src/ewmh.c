#include "ewmh.h"

void
ewmh_init(Display *dpy, Window root)
{
 	Atom atoms[] = {
		atom(NetClientList),
		atom(NetSupported),
	};

	XChangeProperty(dpy, root, atom(NetSupported), XA_ATOM, 32,
			PropModeReplace, (unsigned char *) atoms,
			ARRSIZE(atoms));
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


