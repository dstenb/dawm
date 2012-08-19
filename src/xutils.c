#include "dawm.h"

static int has_wm_protocol(Window, Atom);

int
has_wm_protocol(Window win, Atom prot)
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

long
get_state(Window win)
{
	int format;
	long result = -1;
	unsigned char *p = NULL;
	unsigned long n, extra;
	Atom real;

	if(XGetWindowProperty(dpy, win, atom(WMState), 0L, 2L, False,
				atom(WMState), &real, &format, &n, &extra,
				(unsigned char **)&p) != Success)
		return -1;
	if(n != 0)
		result = *p;
	XFree(p);
	return result;
}

bool
send_event(Window win, Atom proto)
{
	XEvent ev;

	if (has_wm_protocol(win, proto)) {
		ev.type = ClientMessage;
		ev.xclient.window = win;
		ev.xclient.message_type = atom(WMProtocols);
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = proto;
		ev.xclient.data.l[1] = CurrentTime;

		XSendEvent(dpy, win, False, NoEventMask, &ev);

		return true;
	} else {
		return false;
	}
}

int
xerror_dummy(Display *_dpy, XErrorEvent *ev)
{
	(void)_dpy;
	(void)ev;
	return 0;
}
