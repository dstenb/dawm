#include "wm.h"

#define WM_EVENT_MASK SubstructureRedirectMask | SubstructureNotifyMask \
	| ButtonPressMask | ButtonReleaseMask | PointerMotionMask | \
	EnterWindowMask | LeaveWindowMask | StructureNotifyMask | \
	PropertyChangeMask

static void wm_checkotherwm(struct wm *);
static void wm_create_client(struct wm *, Window, XWindowAttributes *);
static void wm_create_monitors(struct wm *);
static void wm_get_windows(struct wm *);
static void wm_keypress(struct wm *, struct key *);
static void wm_quit(struct wm *, const char *);
static void wm_remove_client(struct wm *, struct client *, int);
static void wm_restart(struct wm *);
static void wm_update_net_client_list(struct wm *);
static int wm_xerror_checkotherwm(Display *, XErrorEvent *);
static int wm_xerror(Display *, XErrorEvent *);

/* X handlers */
static void wm_handler_buttonpress(struct wm *, XEvent *);
static void wm_handler_buttonrelease(struct wm *, XEvent *);
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
	[ButtonRelease] = wm_handler_buttonrelease,
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

/* TODO: test code, to be removed */
static XButtonEvent start;
static XWindowAttributes move_res_attr;

static void dbg_print(struct wm *wm, const char *str)
{
	DBG(":::: dbg_print(%s)\n", str);

	monitor_dbg_print(wm->selmon, "");
}

void wm_checkotherwm(struct wm *wm)
{
	xerrxlib = XSetErrorHandler(wm_xerror_checkotherwm);
	/* this causes an error if some other window manager is running */
	XSelectInput(wm->dpy, wm->root, SubstructureRedirectMask);
	XSync(wm->dpy, False);
	XSetErrorHandler(wm_xerror);
	XSync(wm->dpy, False);
}

void wm_create_client(struct wm *wm, Window win, XWindowAttributes *attr)
{
	struct client *c = client_create(win, attr);

	client_setup(c, wm->cfg, wm->selmon, wm->dpy, wm->root, attr);

	monitor_add_client(c->mon, c);
	monitor_select_client(c->mon, c);

	client_map_window(c, wm->dpy);

	monitor_arrange(c->mon, wm->dpy);
	monitor_focus(c->mon, c, wm->dpy, wm->root);
}

/* TODO: fix Xinerama */
void wm_create_monitors(struct wm *wm)
{
	wm->mons = monitor_create(wm->cfg, wm->width, wm->height);
	wm->selmon = wm->mons;
}

int wm_destroy(struct wm *wm)
{
	config_free(wm->cfg);

	XUngrabKey(wm->dpy, AnyKey, AnyModifier, wm->root);
	XCloseDisplay(wm->dpy);
	free(wm);
	return 0;
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

	/* select events to handle */
	attr.event_mask = WM_EVENT_MASK;
	XChangeWindowAttributes(wm->dpy, wm->root, CWEventMask, &attr);

	DBG("wm->width: %i\n", wm->width);
	DBG("wm->height: %i\n", wm->height);

	/* grab the manager's key bindings */
	key_grab_all(wm->keys, wm->dpy, wm->root);

	/* TODO: to be removed */
	XGrabButton(wm->dpy, 1, Mod1Mask, wm->root, True, ButtonPressMask, GrabModeAsync,
			GrabModeAsync, None, None);
	XGrabButton(wm->dpy, 3, Mod1Mask, wm->root, True, ButtonPressMask, GrabModeAsync,
			GrabModeAsync, None, None);

	clients_init_colors(wm->cfg, wm->dpy, wm->screen);

	atoms_init(wm->dpy);

	/* manage all windows that already exists */
	wm_get_windows(wm);

	return wm;
}

void wm_get_windows(struct wm *wm)
{
	XWindowAttributes attr;
	Window d1, d2;
	Window *wins;
	unsigned int i, n;

	if (XQueryTree(wm->dpy, wm->root, &d1, &d2, &wins, &n)) {
		for (i = 0; i < n; i++) {
			if(!XGetWindowAttributes(wm->dpy, wins[i], &attr) ||
					attr.override_redirect)
				continue;

			/* TODO: fix hints */
			wm_create_client(wm, wins[i], &attr);
		}

		if (wins)
			XFree(wins);
	}
}

void wm_handler_buttonpress(struct wm *wm, XEvent *ev)
{
	XButtonPressedEvent *bpev = &ev->xbutton;
	struct client *c;

	dbg_print(wm, __func__);

	if (!(c = find_client_by_window(wm->mons, bpev->subwindow)))
		return;

	/* TODO: test code, to be removed */
	XGrabPointer(wm->dpy, ev->xbutton.subwindow, True,
			PointerMotionMask|ButtonReleaseMask, GrabModeAsync,
			GrabModeAsync, None, None, CurrentTime);
	XGetWindowAttributes(wm->dpy, ev->xbutton.subwindow, &move_res_attr);
	start = ev->xbutton;
}

void wm_handler_buttonrelease(struct wm *wm, XEvent *ev)
{
	(void)ev;

	dbg_print(wm, __func__);

	XUngrabPointer(wm->dpy, CurrentTime);
}

void wm_handler_clientmessage(struct wm *wm, XEvent *ev)
{
	XClientMessageEvent *cmev = &ev->xclient;

	dbg_print(wm, __func__);

	if (cmev->window == wm->root) {
		if (cmev->format == 32) {
			/* TODO: Maybe handle
			 * net_current_desktop & net_number_of_desktops */
		}
	} else {
		struct client *c;

		if (!(c = find_client_by_window(wm->mons, cmev->window)))
			return;

		if (cmev->message_type == atom(WMStateAtom)) {
			DBG("%s: WMState", __func__);
		} else if (cmev->message_type == atom(NetActiveWindowAtom)) {
			DBG("%s: NetActiveWindow", __func__);
		} else if (cmev->message_type == atom(WMChangeStateAtom)) {
			if (cmev->data.l[0] == IconicState &&
					cmev->format == 32) {
				DBG("%s: minimize window\n", __func__);
				/* TODO: fix minimizing */
			}
		}
	}
}

void wm_handler_configurerequest(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	dbg_print(wm, __func__);
}

void wm_handler_configurenotify(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	dbg_print(wm, __func__);
}

void wm_handler_destroynotify(struct wm *wm, XEvent *ev)
{
	struct client *c;
	Window win;

	dbg_print(wm, __func__);

	win = ev->xdestroywindow.window;
	if ((c = find_client_by_window(wm->mons, win)))
		wm_remove_client(wm, c, 1);
}

void wm_handler_enternotify(struct wm *wm, XEvent *ev)
{
	XCrossingEvent *cev = &ev->xcrossing;
	struct client *c;

	dbg_print(wm, __func__);

	if (!(c = find_client_by_window(wm->mons, cev->window)))
		return;

	monitor_focus(c->mon, c, wm->dpy, wm->root);
}

void wm_handler_expose(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	dbg_print(wm, __func__);
}

void wm_handler_focusin(struct wm *wm, XEvent *ev)
{
	XFocusChangeEvent *fcev = &ev->xfocus;

	dbg_print(wm, __func__);

	/* reacquire focus from a broken client */
	if(wm->selmon->sel && fcev->window != wm->selmon->sel->win)
		monitor_focus(wm->selmon, wm->selmon->sel, wm->dpy, wm->root);
}

/* TODO: handle keys without a modifier mask */
void wm_handler_keypress(struct wm *wm, XEvent *ev)
{
	XKeyEvent *kev;
	struct key *key;

	dbg_print(wm, __func__);

	kev = &ev->xkey;
	for (key = wm->keys; key; key = key->next) {
		if (XKeysymToKeycode(wm->dpy, key->keysym) == kev->keycode
				&& (kev->state & key->mod))
			wm_keypress(wm, key);
	}
}

void wm_handler_mappingnotify(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	dbg_print(wm, __func__);
}

void wm_handler_maprequest(struct wm *wm, XEvent *ev)
{
	static XWindowAttributes attr;
	XMapRequestEvent *mrev = &ev->xmaprequest;
	
	dbg_print(wm, __func__);

	if(!XGetWindowAttributes(wm->dpy, mrev->window, &attr) ||
			attr.override_redirect)
		return;
	if(!find_client_by_window(wm->mons, mrev->window))
		wm_create_client(wm, mrev->window, &attr);
}

void wm_handler_motionnotify(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;

	/* TODO: test code, to be removed */
	int xdiff, ydiff;
	while(XCheckTypedEvent(wm->dpy, MotionNotify, ev));
	xdiff = ev->xbutton.x_root - start.x_root;
	ydiff = ev->xbutton.y_root - start.y_root;
	XMoveResizeWindow(wm->dpy, ev->xmotion.window,
			move_res_attr.x + (start.button==1 ? xdiff : 0),
			move_res_attr.y + (start.button==1 ? ydiff : 0),
			MAX(1, move_res_attr.width + (start.button==3 ? xdiff : 0)),
			MAX(1, move_res_attr.height + (start.button==3 ? ydiff : 0)));
}

void wm_handler_propertynotify(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;

	dbg_print(wm, __func__);

	/* TODO: check for XA_WM_NAME property change */

	/* TODO: check for client property change */
}

void wm_handler_unmapnotify(struct wm *wm, XEvent *ev)
{
	struct client *c;
	XUnmapEvent *uev = &ev->xunmap;
	
	dbg_print(wm, __func__);

	if((c = find_client_by_window(wm->mons, uev->window))) {
		if(uev->send_event)
			client_set_state(c, wm->dpy, WithdrawnState);
		else
			wm_remove_client(wm, c, 0);
	}
}

void wm_keypress(struct wm *wm, struct key *key)
{
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
			die("unhandled key action (%d), fix this!\n",
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

	monitor_arrange(mon, wm->dpy);

	wm_update_net_client_list(wm);

	/* TODO: focus */
}

void wm_restart(struct wm *wm)
{
	DBG("%s. restarting!\n", __func__);
	if (wm->cmd)
		execlp("/bin/sh", "sh" , "-c", wm->cmd, NULL);
}

void wm_update_net_client_list(struct wm *wm)
{
	struct monitor *mon;
	struct client *c;

	net_client_list_clear(wm->dpy, wm->root);

	for (mon = wm->mons; mon; mon = mon->next)
		for (c = mon->clients; c; c = c->next)
			net_client_list_add(wm->dpy, wm->root, c->win);
}

int wm_xerror(Display *dpy, XErrorEvent *ee)
{
	error("fatal error: request code=%d, error code=%d\n",
			ee->request_code, ee->error_code);
	return 0; /* FIXME: handle/ignore errors in a proper way */
	return xerrxlib(dpy, ee); /* may call exit */
}

int wm_xerror_checkotherwm(Display *dpy, XErrorEvent *ee)
{
	(void)dpy;
	(void)ee;
	die("another window manager is running\n");
	return 0;
}
