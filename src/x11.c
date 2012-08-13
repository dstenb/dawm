#include "dawm.h"

static bool initialized = false;

Display *dpy;
Window root;
int dpy_fd;
int screen;
int screen_w;
int screen_h;

void
x11_init(void)
{
	if (!initialized) {
		if (!(dpy = XOpenDisplay(NULL)))
			die("couldn't open display '%s'\n", getenv("DISPLAY"));

		screen = DefaultScreen(dpy);
		root = RootWindow(dpy, screen);
		dpy_fd = ConnectionNumber(dpy);
		screen_w = DisplayWidth(dpy, screen);
		screen_h = DisplayHeight(dpy, screen);

		initialized = true;
	}
}

void
x11_destroy(void)
{
	if (initialized) {
		XUngrabKey(dpy, AnyKey, AnyModifier, root);
		XSync(dpy, False);
		XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot,
				CurrentTime);
		XCloseDisplay(dpy);
	}
}
