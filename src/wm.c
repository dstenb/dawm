#include "wm.h"

#define WM_EVENT_MASK SubstructureRedirectMask | SubstructureNotifyMask \
	| ButtonPressMask | ButtonReleaseMask | PointerMotionMask | \
	EnterWindowMask | LeaveWindowMask | StructureNotifyMask | \
	PropertyChangeMask

static void wm_checkotherwm(struct wm *);
static void wm_create_client(struct wm *, Window, XWindowAttributes *);
static void wm_create_monitors(struct wm *);
static void wm_get_windows(struct wm *);
static void wm_handler_propertynotify_client(struct wm *, XPropertyEvent *);
static void wm_handler_propertynotify_root(struct wm *, XPropertyEvent *);
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

	monitor_unfocus_selected(c->mon, wm->dpy, wm->root);
	monitor_select_client(c->mon, c);

	client_map_window(c, wm->dpy);

	monitor_arrange(c->mon, wm->dpy);
	monitor_focus(c->mon, c, wm->dpy, wm->root);
}

/* TODO: fix Xinerama */
void wm_create_monitors(struct wm *wm)
{
	/* TODO: test code, to be removed */

	wm->mons = monitor_create(wm->cfg, 0, 0, wm->width / 2, wm->height, wm->dpy,
			wm->root, wm->screen);
	/*wm->mons->next = monitor_create(wm->cfg, wm->width / 2, 0,
			wm->width/2, wm->height, wm->dpy,
			wm->root, wm->screen);*/
	wm->selmon = wm->mons;
}

int wm_destroy(struct wm *wm)
{
	/* TODO */

	config_free(wm->cfg);

	XUngrabKey(wm->dpy, AnyKey, AnyModifier, wm->root);
	cursors_free(wm->dpy);
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
	wm->motion.type = NoMotion;

	colors_init(cfg->colors, wm->dpy, wm->screen);

	wm_create_monitors(wm);

	/* select events to handle */
	attr.event_mask = WM_EVENT_MASK;
	XChangeWindowAttributes(wm->dpy, wm->root, CWEventMask, &attr);

	DBG("wm->width: %i\n", wm->width);
	DBG("wm->height: %i\n", wm->height);

	/* setup cursors */
	cursors_init(wm->dpy);
	attr.cursor = cursor(NormalCursor);
	XChangeWindowAttributes(wm->dpy, wm->root, CWCursor, &attr);

	/* grab the manager's key bindings */
	update_num_lock(wm->dpy);
	key_grab_all(wm->keys, wm->dpy, wm->root);

	atoms_init(wm->dpy);
	net_set_supported(wm->dpy, wm->root);

	/* manage all windows that already exists */
	wm_get_windows(wm);

	set_text_prop(wm->dpy, wm->root, atom(NetWMName), WMNAME);

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

	if (!(c = find_client_by_window(wm->mons, bpev->window)))
		return;

	XGrabPointer(wm->dpy, ev->xbutton.window, True,
			PointerMotionMask | ButtonReleaseMask, GrabModeAsync,
			GrabModeAsync, None, None, CurrentTime);

	/* set start motion data */
	XGetWindowAttributes(wm->dpy, ev->xbutton.window, &wm->motion.attr);
	wm->motion.start = ev->xbutton;

	/* set the motion type */
	if (wm->motion.start.button == 1)
		wm->motion.type = MovementMotion;
	else if (wm->motion.start.button == 3)
		wm->motion.type = ResizeMotion;
}

void wm_handler_buttonrelease(struct wm *wm, XEvent *ev)
{
	(void)ev;

	dbg_print(wm, __func__);

	wm->motion.type = NoMotion;
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

		if (cmev->message_type == atom(WMState)) {
			DBG("%s: WMState", __func__);
		} else if (cmev->message_type == atom(NetActiveWindow)) {
			DBG("%s: NetActiveWindow", __func__);
		} else if (cmev->message_type == atom(WMChangeState)) {
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
	XConfigureRequestEvent *crev = &ev->xconfigurerequest;
	struct client *c;

	dbg_print(wm, __func__);

	if ((c = find_client_by_window(wm->mons, crev->window))) {
		if (crev->value_mask & CWBorderWidth) {
			/* TODO: set border size (crev->border_width) */
			DBG("%s: set window border\n", __func__);
		} else {
			/* TODO: set window size if the client is floating or
			 * non-arranged */
			DBG("%s: set window size\n", __func__);
		}
	} else {
		/* TODO: send XConfigureWindow to window */
		DBG("%s: send XConfigureWindow\n");
	}

	XSync(wm->dpy, False);
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

void wm_handler_keypress(struct wm *wm, XEvent *ev)
{
	XKeyEvent *kev;
	struct key *key;

	dbg_print(wm, __func__);

	kev = &ev->xkey;
	for (key = wm->keys; key; key = key->next) {
		if (key_pressed(key, wm->dpy, kev->keycode, kev->state))
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
	XMotionEvent *mev = &ev->xmotion;
	struct client *c;

	int was_floating = 0;
	int xdiff, ydiff;
	int x, y, w, h;

	if (!(c = find_client_by_window(wm->mons, mev->window)))
		return;

	x = c->cur_r.x;
	y = c->cur_r.y;
	w = c->cur_r.w;
	h = c->cur_r.h;

	while(XCheckTypedEvent(wm->dpy, MotionNotify, ev));

	xdiff = mev->x_root - wm->motion.start.x_root;
	ydiff = mev->y_root - wm->motion.start.y_root;

	if (wm->motion.type == ResizeMotion) {
		if (!c->floating) { /* TODO: check if the monitor is arranged */
			w = c->old_r.w;
			h = c->old_r.h;
		} else {
			w = wm->motion.attr.width + xdiff;
			h = wm->motion.attr.height + ydiff;
		}
	} else if (wm->motion.type == MovementMotion) {
		x = wm->motion.attr.x + xdiff;
		y = wm->motion.attr.y + ydiff;
	}

	/* TODO: check if the monitor is arranged. */
	was_floating = c->floating;
	c->floating = 1;
	client_raise(c, wm->dpy);

	client_move_resize(c, wm->dpy, x, y, w, h);

	/* re-arrange the monitor's client if the selected client
	 * wasn't floating before */
	if (!was_floating)
		monitor_arrange(c->mon, wm->dpy);
}

void wm_handler_propertynotify(struct wm *wm, XEvent *ev)
{
	XPropertyEvent *pev = &ev->xproperty;

	dbg_print(wm, __func__);

	if (pev->window == wm->root)
		wm_handler_propertynotify_root(wm, pev);
	else
		wm_handler_propertynotify_client(wm, pev);

}

void wm_handler_propertynotify_client(struct wm *wm, XPropertyEvent *ev)
{
	struct client *c;

	if (!(c = find_client_by_window(wm->mons, ev->window)))
		return;

	if (ev->atom == XA_WM_NAME || ev->atom == atom(NetWMName)) {
		client_update_title(c, wm->dpy);
		/* TODO: redraw the bar */
	} else if (ev->atom == XA_WM_TRANSIENT_FOR) {
		/* TODO */
		DBG("transient\n");
	} else if (ev->atom == XA_WM_NORMAL_HINTS) {
		/* TODO */
		DBG("normal hints\n");
	} else if (ev->atom == XA_WM_HINTS) {
		/* TODO: check for fullscreen and floating */
		DBG("hints\n");
	} else if (ev->atom == atom(NetWMWindowType)) {
		/* TODO */
		DBG("window type\n");
	}
}

void wm_handler_propertynotify_root(struct wm *wm, XPropertyEvent *ev)
{
	(void)wm;

	if (ev->atom == XA_WM_NAME) {
		DBG("wm name");
		set_text_prop(wm->dpy, wm->root, atom(NetWMName), WMNAME);
	}
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
		case KillAction:
			client_kill(wm->selmon->sel, wm->dpy);
			break;
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

	monitor_focus(mon, NULL, wm->dpy, wm->root);
	monitor_arrange(mon, wm->dpy);

	wm_update_net_client_list(wm);
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
