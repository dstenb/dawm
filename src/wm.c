#include "wm.h"

#define DEFAULT_EVENT_MASK SubstructureRedirectMask | SubstructureNotifyMask \
	| ButtonPressMask | PointerMotionMask | EnterWindowMask \
	| LeaveWindowMask | StructureNotifyMask | PropertyChangeMask

static unsigned long wm_getcolor(struct wm *, const char *);
static void wm_checkotherwm(struct wm *);
static int wm_xerror_checkotherwm(Display *, XErrorEvent *);
static int wm_xerror(Display *, XErrorEvent *);
static void wm_handler_keypress(struct wm *, XEvent *);
static void wm_handler_buttonpress(struct wm *, XEvent *);
static void wm_handler_motionnotify(struct wm *, XEvent *);

static int (*xerrxlib) (Display *, XErrorEvent *);

void wm_checkotherwm(struct wm *wm)
{
	xerrxlib = XSetErrorHandler(wm_xerror_checkotherwm);
	/* this causes an error if some other window manager is running */
	XSelectInput(wm->dpy, wm->root, SubstructureRedirectMask);
	XSync(wm->dpy, False);
	XSetErrorHandler(wm_xerror);
	XSync(wm->dpy, False);
}

int wm_xerror(Display *dpy, XErrorEvent *ee)
{
	error("fatal error: request code=%d, error code=%d\n",
			ee->request_code, ee->error_code);
	return xerrxlib(dpy, ee); /* may call exit */

}

int wm_xerror_checkotherwm(Display *dpy, XErrorEvent *ee)
{
	die("another window manager is running\n");
	return 0;
}

unsigned long wm_getcolor(struct wm *wm, const char *str) {
	Colormap cmap = DefaultColormap(wm->dpy, wm->screen);
	XColor color;

	if(!XAllocNamedColor(wm->dpy, cmap, str, &color, &color))
		die("error, cannot allocate color '%s'\n", str);
	return color.pixel;
}

struct wm *wm_init(void)
{
	XSetWindowAttributes attr;
	struct wm *wm;

	if (!(wm = calloc(1, sizeof(struct wm))))
		die("couldn't malloc\n");

	if (!(wm->dpy = XOpenDisplay(NULL)))
		die("couldn't open display '%s'\n", getenv("DISPLAY"));

	wm->screen = DefaultScreen(wm->dpy);
	wm->root = RootWindow(wm->dpy, wm->screen);

	/* check if another wm is running */
	wm_checkotherwm(wm);

	wm->width = DisplayWidth(wm->dpy, wm->screen);
	wm->height = DisplayHeight(wm->dpy, wm->screen);
	wm->restart = 0;

	attr.event_mask = DEFAULT_EVENT_MASK;
	XChangeWindowAttributes(wm->dpy, wm->root, CWEventMask, &attr);

	printf("wm->width: %i\n", wm->width);
	printf("wm->height: %i\n", wm->height);

    XGrabKey(wm->dpy, XKeysymToKeycode(wm->dpy, XStringToKeysym("F1")), Mod1Mask, wm->root,
            True, GrabModeAsync, GrabModeAsync);
    XGrabButton(wm->dpy, 1, Mod1Mask, wm->root, True, ButtonPressMask, GrabModeAsync,
            GrabModeAsync, None, None);
    XGrabButton(wm->dpy, 3, Mod1Mask, wm->root, True, ButtonPressMask, GrabModeAsync,
            GrabModeAsync, None, None);

	return wm;
}

int wm_eventloop(struct wm *wm)
{
	XEvent ev;

	for (;;) {
		XNextEvent(wm->dpy, &ev);
		error("ABC\n");

		switch(ev.type) {
			case KeyPress:
				wm_handler_keypress(wm, &ev);
				break;
			case ButtonPress:
				wm_handler_buttonpress(wm, &ev);
				break;
			case MotionNotify:
				wm_handler_motionnotify(wm, &ev);
				break;
			default:
				break;
		}
	}

	(void)wm;
	return 0;
}

int wm_destroy(struct wm *wm)
{
	free(wm);
	return 0;
}

void wm_handler_keypress(struct wm *wm, XEvent *ev)
{
	error("keypress handler");
}

void wm_handler_buttonpress(struct wm *wm, XEvent *ev)
{
	error("buttonpress handler");
}

void wm_handler_motionnotify(struct wm *wm, XEvent *ev)
{
	error("motionnotify handler");
}


