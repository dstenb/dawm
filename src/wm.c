#include "wm.h"

#define WM_EVENT_MASK SubstructureRedirectMask | SubstructureNotifyMask \
	| ButtonPressMask | PointerMotionMask | EnterWindowMask \
	| LeaveWindowMask | StructureNotifyMask | PropertyChangeMask

static void wm_checkotherwm(struct wm *);
static void wm_create_client(struct wm *, Window, XWindowAttributes *);
static void wm_create_monitors(struct wm *);
static void wm_grab_keys(struct wm *);
static void wm_keypress(struct wm *, struct key *);
static void wm_quit(struct wm *, const char *);
static void wm_remove_client(struct wm *, struct client *, int);
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

void wm_create_client(struct wm *wm, Window win, XWindowAttributes *wa)
{
	struct client *c = client_create(win, wa);

	c->mon = wm->selmon;

	client_update_title(c, wm->dpy);

	/* TODO: fix client rules (rule.h) */

	c->old_bsize = wa->border_width;
	client_set_border(c, wm->dpy, wm->cfg->bsize);

	/* TODO: configureevent */
	/*client_fix_window_type(c);*/
	/* TODO: fix size & wm hints */

	client_select_input(c, wm->dpy);
	client_grab_buttons(c, wm->dpy);

	client_raise(c, wm->dpy);

	monitor_add_client(c->mon, c);
	monitor_select_client(c->mon, c);

	XMapWindow(wm->dpy, c->win);

	monitor_arrange(c->mon);
	monitor_focus(c->mon, c, wm->dpy, wm->root);
}

/* TODO: fix Xinerama */
void wm_create_monitors(struct wm *wm)
{
	wm->mons = monitor_create(wm->cfg, wm->width, wm->height);
	wm->selmon = wm->mons;
}

struct wm *wm_init(struct config *cfg, const char *cmd)
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
	wm->keys = cfg->keys;
	wm->cfg = cfg;

	wm_create_monitors(wm);

	attr.event_mask = WM_EVENT_MASK;
	XChangeWindowAttributes(wm->dpy, wm->root, CWEventMask, &attr);

	printf("wm->width: %i\n", wm->width);
	printf("wm->height: %i\n", wm->height);

	/* grab the manager's key bindings */
	wm_grab_keys(wm);

	/* TODO: to be removed */
	XGrabButton(wm->dpy, 1, Mod1Mask, wm->root, True, ButtonPressMask, GrabModeAsync,
			GrabModeAsync, None, None);
	XGrabButton(wm->dpy, 3, Mod1Mask, wm->root, True, ButtonPressMask, GrabModeAsync,
			GrabModeAsync, None, None);

	clients_init_colors(wm->cfg, wm->dpy, wm->screen);

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
	config_free(wm->cfg);

	XUngrabKey(wm->dpy, AnyKey, AnyModifier, wm->root);
	XCloseDisplay(wm->dpy);
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
	XButtonPressedEvent *bpev = &ev->xbutton;
	struct client *c;

	c = find_client_by_window(wm->mons, bpev->window);

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
	struct client *c;
	Window win;

	win = ev->xdestroywindow.window;
	if ((c = find_client_by_window(wm->mons, win)))
		wm_remove_client(wm, c, 1);
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
	XFocusChangeEvent *fcev = &ev->xfocus;

	/* reacquire focus from a broken client */
	if(wm->selmon->sel && fcev->window != wm->selmon->sel->win)
		monitor_focus(wm->selmon, wm->selmon->sel, wm->dpy, wm->root);
	
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
	static XWindowAttributes wa;
	XMapRequestEvent *mrev = &ev->xmaprequest;

	if(!XGetWindowAttributes(wm->dpy, mrev->window, &wa))
		return;
	if(wa.override_redirect)
		return;
	if(!find_client_by_window(wm->mons, mrev->window))
		wm_create_client(wm, mrev->window, &wa);

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

	/* TODO: check for XA_WM_NAME property change */

	/* TODO: check for client property change */
}

void wm_handler_unmapnotify(struct wm *wm, XEvent *ev)
{
	struct client *c;
	XUnmapEvent *uev = &ev->xunmap;

	if((c = find_client_by_window(wm->mons, uev->window))) {
		if(uev->send_event)
			client_set_state(c, wm->dpy, WithdrawnState);
		else
			wm_remove_client(wm, c, 0);
	}

	error("%s\n", __func__);
}

void wm_keypress(struct wm *wm, struct key *key)
{
	error("%s\n", __func__);

	switch(key->action) {
		case SpawnAction:
			spawn(key->args);
			break;
		case QuitAction:
			wm_quit(wm, "received exit key command");
			break;
		case RestartAction:
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
	wm_destroy(wm);
	if (reason)
		die("quitting (%s)\n", reason);
	die("quitting\n");
}

void wm_remove_client(struct wm *wm, struct client *c, int destroyed)
{
	struct monitor *mon = c->mon;

	monitor_remove_client(mon, c);

	if (!destroyed)
		client_unmap(c, wm->dpy);

	client_free(c);

	monitor_arrange(mon);
	/* TODO: focus, fix client list */
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
