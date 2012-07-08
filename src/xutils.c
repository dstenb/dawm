#include "xutils.h"

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
