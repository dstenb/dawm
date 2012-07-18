#include "xutils.h"

static int has_wm_protocol(Display *, Window, Atom);

int
has_wm_protocol(Display *dpy, Window win, Atom prot)
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
get_state(Display *dpy, Window win)
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

int
send_event(Display *dpy, Window win, Atom proto)
{
	XEvent ev;

	if (has_wm_protocol(dpy, win, proto)) {
		ev.type = ClientMessage;
		ev.xclient.window = win;
		ev.xclient.message_type = atom(WMProtocols);
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = proto;
		ev.xclient.data.l[1] = CurrentTime;

		XSendEvent(dpy, win, False, NoEventMask, &ev);

		return 1;
	} else {
		return 0;
	}
}

int
xerror_dummy(Display *dpy, XErrorEvent *ev)
{
	(void)dpy;
	(void)ev;
	return 0;
}
