#include "wm.h"

#define DEFAULT_EVENT_MASK SubstructureRedirectMask | SubstructureNotifyMask \
	| ButtonPressMask | PointerMotionMask | EnterWindowMask \
	| LeaveWindowMask | StructureNotifyMask | PropertyChangeMask

static unsigned long wm_getcolor(struct wm *, const char *);
static void wm_checkotherwm(struct wm *);
static void wm_grab_keys(struct wm *);
static void wm_keypress(struct wm *, struct key *);
static void wm_quit(struct wm *, const char *);
static void wm_restart(struct wm *);
static int wm_xerror_checkotherwm(Display *, XErrorEvent *);
static int wm_xerror(Display *, XErrorEvent *);

/* X handlers */
static void wm_handler_buttonpress(struct wm *, XEvent *);
static void wm_handler_clientmessage(struct wm *, XEvent *);
static void wm_handler_configurerequest(struct wm *, XEvent *);
static void wm_handler_configurenotify(struct wm *, XEvent *);
static void wm_handler_destroynotify(struct wm *, XEvent *);
static void wm_handler_enternotify(struct wm *, XEvent *);
static void wm_handler_expose(struct wm *, XEvent *);
static void wm_handler_focusin(struct wm *, XEvent *);
static void wm_handler_keypress(struct wm *, XEvent *);
static void wm_handler_mappingnotify(struct wm *, XEvent *);
static void wm_handler_maprequest(struct wm *, XEvent *);
static void wm_handler_motionnotify(struct wm *, XEvent *);
static void wm_handler_propertynotify(struct wm *, XEvent *);
static void wm_handler_unmapnotify(struct wm *, XEvent *);

static int (*xerrxlib) (Display *, XErrorEvent *);
static void (*handler[LASTEvent]) (struct wm *, XEvent *) = {
	[ButtonPress] = wm_handler_buttonpress,
	[ClientMessage] = wm_handler_clientmessage,
	[ConfigureRequest] = wm_handler_configurerequest,
	[ConfigureNotify] = wm_handler_configurenotify,
	[DestroyNotify] = wm_handler_destroynotify,
	[EnterNotify] = wm_handler_enternotify,
	[Expose] = wm_handler_expose,
	[FocusIn] = wm_handler_focusin,
	[KeyPress] = wm_handler_keypress,
	[MappingNotify] = wm_handler_mappingnotify,
	[MapRequest] = wm_handler_maprequest,
	[MotionNotify] = wm_handler_motionnotify,
	[PropertyNotify] = wm_handler_propertynotify,
	[UnmapNotify] = wm_handler_unmapnotify
};

void wm_checkotherwm(struct wm *wm)
{
	xerrxlib = XSetErrorHandler(wm_xerror_checkotherwm);
	/* this causes an error if some other window manager is running */
	XSelectInput(wm->dpy, wm->root, SubstructureRedirectMask);
	XSync(wm->dpy, False);
	XSetErrorHandler(wm_xerror);
	XSync(wm->dpy, False);
}

unsigned long wm_getcolor(struct wm *wm, const char *str)
{
	Colormap cmap = DefaultColormap(wm->dpy, wm->screen);
	XColor color;

	if(!XAllocNamedColor(wm->dpy, cmap, str, &color, &color))
		die("error, cannot allocate color '%s'\n", str);
	return color.pixel;
}

struct wm *wm_init(const char *cmd)
{
	XSetWindowAttributes attr;
	struct wm *wm;

	wm = xcalloc(1, sizeof(struct wm));

	if (!(wm->dpy = XOpenDisplay(NULL)))
		die("couldn't open display '%s'\n", getenv("DISPLAY"));

	wm->screen = DefaultScreen(wm->dpy);
	wm->root = RootWindow(wm->dpy, wm->screen);

	/* check if another wm is running */
	wm_checkotherwm(wm);

	wm->width = DisplayWidth(wm->dpy, wm->screen);
	wm->height = DisplayHeight(wm->dpy, wm->screen);
	wm->cmd = cmd;

	attr.event_mask = DEFAULT_EVENT_MASK;
	XChangeWindowAttributes(wm->dpy, wm->root, CWEventMask, &attr);

	printf("wm->width: %i\n", wm->width);
	printf("wm->height: %i\n", wm->height);

	/* TODO: remove this and add keys as an argument to the init func. */
	wm->keys = key_default_bindings();

	/* grab the manager's key bindings */
	wm_grab_keys(wm);

	/* TODO: to be removed */
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
		if (handler[ev.type])
			handler[ev.type](wm, &ev);
	}
	return 0;
}

int wm_destroy(struct wm *wm)
{
	free(wm);
	return 0;
}

void wm_grab_keys(struct wm *wm)
{
	struct key *key;

	for (key = wm->keys; key; key = key->next) {
		XGrabKey(wm->dpy, XKeysymToKeycode(wm->dpy, key->keysym),
				key->mod, wm->root, True, GrabModeAsync,
				GrabModeAsync);
	}
}

void wm_handler_buttonpress(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	error("%s\n", __func__);
}

void wm_handler_clientmessage(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	error("%s\n", __func__);
}

void wm_handler_configurerequest(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	error("%s\n", __func__);
}

void wm_handler_configurenotify(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	error("%s\n", __func__);
}

void wm_handler_destroynotify(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	error("%s\n", __func__);
}

void wm_handler_enternotify(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	error("%s\n", __func__);
}

void wm_handler_expose(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	error("%s\n", __func__);
}

void wm_handler_focusin(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	error("%s\n", __func__);
}

/* TODO cleanup */
void wm_handler_keypress(struct wm *wm, XEvent *ev)
{
	XKeyEvent *kev;
	struct key *key;

	kev = &ev->xkey;
	for (key = wm->keys; key; key = key->next) {
		if (XKeysymToKeycode(wm->dpy, key->keysym) == kev->keycode
				&& (kev->state & key->mod))
			wm_keypress(wm, key);
	}

	error("%s\n", __func__);
}

void wm_handler_mappingnotify(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	error("%s\n", __func__);
}

void wm_handler_maprequest(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	error("%s\n", __func__);
}

void wm_handler_motionnotify(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	error("%s\n", __func__);
}

void wm_handler_propertynotify(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	error("%s\n", __func__);
}

void wm_handler_unmapnotify(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	error("%s\n", __func__);
}

void wm_keypress(struct wm *wm, struct key *key)
{
	error("%s\n", __func__);

	switch(key->action) {
		case SPAWN:
			printf("%s: spawn(%s)!\n", __func__, key->args);
			/* TODO */
			break;
		case QUIT:
			wm_quit(wm, "received exit key command");
			break;
		case RESTART:
			wm_restart(wm);
			break;
		default:
			error("unhandled key action (%d), fix this!\n",
					key->action);
			break;
	}
}

void wm_quit(struct wm *wm, const char *reason)
{
	(void)wm;
	if (reason)
		die("quitting (%s)\n", reason);
	die("quitting\n");
}

void wm_restart(struct wm *wm)
{
	printf("%s. restarting!\n", __func__);
	if (wm->cmd)
		execlp("/bin/sh", "sh" , "-c", wm->cmd, NULL);
}

int wm_xerror(Display *dpy, XErrorEvent *ee)
{
	error("fatal error: request code=%d, error code=%d\n",
			ee->request_code, ee->error_code);
	return xerrxlib(dpy, ee); /* may call exit */
}

int wm_xerror_checkotherwm(Display *dpy, XErrorEvent *ee)
{
	(void)dpy;
	(void)ee;
	die("another window manager is running\n");
	return 0;
}
