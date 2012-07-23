#include "wm.h"

#define WM_EVENT_MASK SubstructureRedirectMask | SubstructureNotifyMask \
	| ButtonPressMask | ButtonReleaseMask | PointerMotionMask | \
	EnterWindowMask | LeaveWindowMask | StructureNotifyMask | \
	PropertyChangeMask

static void checkotherwm(struct wm *);
static void create_client(struct wm *, Window, XWindowAttributes *);
static void create_monitors(struct wm *);
static void get_windows(struct wm *);
static void handler_propertynotify_client(struct wm *, XPropertyEvent *);
static void handler_propertynotify_root(struct wm *, XPropertyEvent *);
static int manageable_window(Display *, Window, XWindowAttributes *, int);
static void quit(struct wm *, const char *);
static void remove_client(struct wm *, struct client *, int);
static void restart(struct wm *);
static void set_environment(struct wm *);
static void set_monitor(struct wm *, struct monitor *);
static void update_bars(struct wm *);
static void update_net_client_list(struct wm *);
static int xerror_checkotherwm(Display *, XErrorEvent *);
static int xerror(Display *, XErrorEvent *);

/* X handlers */
static void handler_buttonpress(struct wm *, XEvent *);
static void handler_buttonrelease(struct wm *, XEvent *);
static void handler_clientmessage(struct wm *, XEvent *);
static void handler_configurerequest(struct wm *, XEvent *);
static void handler_configurenotify(struct wm *, XEvent *);
static void handler_destroynotify(struct wm *, XEvent *);
static void handler_enternotify(struct wm *, XEvent *);
static void handler_expose(struct wm *, XEvent *);
static void handler_focusin(struct wm *, XEvent *);
static void handler_keypress(struct wm *, XEvent *);
static void handler_mappingnotify(struct wm *, XEvent *);
static void handler_maprequest(struct wm *, XEvent *);
static void handler_motionnotify(struct wm *, XEvent *);
static void handler_propertynotify(struct wm *, XEvent *);
static void handler_unmapnotify(struct wm *, XEvent *);

/* key action handlers */
static void key_handler_kill(struct wm *, struct key *);
static void key_handler_movewindow(struct wm *, struct key *);
static void key_handler_quit(struct wm *, struct key *);
static void key_handler_restart(struct wm *, struct key *);
static void key_handler_select(struct wm *, struct key *);
static void key_handler_setlayout(struct wm *, struct key *);
static void key_handler_setmaster(struct wm *, struct key *);
static void key_handler_setmfact(struct wm *, struct key *);
static void key_handler_setmnum(struct wm *, struct key *);
static void key_handler_setws(struct wm *, struct key *);
static void key_handler_spawn(struct wm *, struct key *);
static void key_handler_swap(struct wm *, struct key *);
static void key_handler_togglebar(struct wm *, struct key *);
static void key_handler_togglefloat(struct wm *, struct key *);
static void key_handler_togglefs(struct wm *, struct key *);

static int (*xerrxlib) (Display *, XErrorEvent *);
static void (*event_handler[LASTEvent]) (struct wm *, XEvent *) = {
	[ButtonPress] = handler_buttonpress,
	[ButtonRelease] = handler_buttonrelease,
	[ClientMessage] = handler_clientmessage,
	[ConfigureRequest] = handler_configurerequest,
	[ConfigureNotify] = handler_configurenotify,
	[DestroyNotify] = handler_destroynotify,
	[EnterNotify] = handler_enternotify,
	[Expose] = handler_expose,
	[FocusIn] = handler_focusin,
	[KeyPress] = handler_keypress,
	[MappingNotify] = handler_mappingnotify,
	[MapRequest] = handler_maprequest,
	[MotionNotify] = handler_motionnotify,
	[PropertyNotify] = handler_propertynotify,
	[UnmapNotify] = handler_unmapnotify
};

static void (*key_handler[LASTAction]) (struct wm *, struct key *) = {
	[KillAction] = key_handler_kill,
	[MoveWindowAction] = key_handler_movewindow,
	[QuitAction] = key_handler_quit,
	[RestartAction] = key_handler_restart,
	[SelectAction] = key_handler_select,
	[SetLayoutAction] = key_handler_setlayout,
	[SetMasterAction] = key_handler_setmaster,
	[SetMasterFactAction] = key_handler_setmfact,
	[SetMasterNumAction] = key_handler_setmnum,
	[SetWsAction] = key_handler_setws,
	[SpawnAction] = key_handler_spawn,
	[SwapAction] = key_handler_swap,
	[ToggleBarAction] = key_handler_togglebar,
	[ToggleFloatAction] = key_handler_togglefloat,
	[ToggleFsAction] = key_handler_togglefs
};

/*#define MONITOR_DBG*/

static void
dbg_print(struct wm *wm, const char *str)
{
	(void)str;
	DBG(":::: dbg_print(%s)\n", str);

#ifdef MONITOR_DBG
	struct monitor *m = wm->selmon;
	struct client *c;

	DBG("m->sel: %p\n", (void *)m->sel);

	DBG("m->clients:\n");
	for (c = m->clients; c; c = c->next)
		DBG("-> %p\n", c);
	DBG("m->cstack:\n");
	for (c = m->cstack; c; c = c->snext)
		DBG("-> %p\n", c);
	DBG("\n");
#else
	(void)wm;
#endif
}

void
checkotherwm(struct wm *wm)
{
	xerrxlib = XSetErrorHandler(xerror_checkotherwm);
	/* this causes an error if some other window manager is running */
	XSelectInput(wm->dpy, wm->root, SubstructureRedirectMask);
	XSync(wm->dpy, False);
	XSetErrorHandler(xerror);
	XSync(wm->dpy, False);
}

void
create_client(struct wm *wm, Window win, XWindowAttributes *attr)
{
	struct client *c, *tc;

	c = client_create(win, attr);

	tc = find_client_by_trans(wm->mons, wm->dpy, win);

	client_setup(c, wm->selmon, wm->mons, wm->dpy, wm->root, tc);
	client_map_window(c, wm->dpy);

	rules_apply(c, wm->dpy);

	monitor_add_client(c->mon, c, wm->dpy, wm->root);
}

void
create_monitors(struct wm *wm)
{
#if 0
	/* TODO: test code, to be removed */
	struct monitor *m;
	wm->mons = monitor_create(0, 0, 0, 1024 / 2, 600, wm->dpy,
			wm->root, wm->screen);
	m = monitor_create(1, 1024 / 2, 0, 1024 / 2, 600, wm->dpy,
			wm->root, wm->screen);
	wm->mons = monitor_append(wm->mons, m);
	wm->selmon = wm->mons;
	return;
#endif
#ifdef XINERAMA
	struct monitor *mon;
	int i, nmon;

	if (XineramaIsActive(wm->dpy)) {
		XineramaScreenInfo *xsi = XineramaQueryScreens(wm->dpy, &nmon);

		wm->mons = NULL;

		for (i = 0; i < nmon; i++) {
			mon = monitor_create(i, xsi[i].x_org, xsi[i].y_org,
					xsi[i].width, xsi[i].height,
					wm->dpy, wm->root, wm->screen);
			wm->mons = monitor_append(wm->mons, mon);
		}

		wm->selmon = wm->mons;
		return;
	}
#endif /* XINERAMA */

	wm->selmon = wm->mons = monitor_create(0, 0, 0, wm->width, wm->height,
			wm->dpy, wm->root, wm->screen);
}

int
destroy(struct wm *wm)
{
	struct monitor *mon;

	for (mon = wm->mons; mon; mon = mon->next) {
		while (mon->cstack)
			remove_client(wm, mon->cstack, 0);
	}

	bars_free(wm->dpy);
	cursors_free(wm->dpy);
	rules_free();

	XUngrabKey(wm->dpy, AnyKey, AnyModifier, wm->root);

	while (wm->mons)
		wm->mons = monitor_free(wm->mons, wm->dpy);

	XSync(wm->dpy, False);
	XSetInputFocus(wm->dpy, PointerRoot, RevertToPointerRoot,
			CurrentTime);
	XCloseDisplay(wm->dpy);
	free(wm);
	return 0;
}

int
eventloop(struct wm *wm)
{
	XEvent ev;
	struct timeval tv;
	fd_set fds;
	int xfd;
	int n;

	/* get display fd */
	xfd = ConnectionNumber(wm->dpy);

	tv.tv_sec = BAR_UPDATE_RATE;
	tv.tv_usec = 0;

	for (;;) {

		FD_ZERO(&fds);
		FD_SET(xfd, &fds);

		if ((n = select(xfd + 1, &fds, NULL, NULL, &tv)) < 0) {
			if (errno == EINTR) /* interrupt during select() */
				continue;
			die("select(): %s\n", strerror(errno));
		} else if (n > 0) {
			while (XPending(wm->dpy)) {
				XNextEvent(wm->dpy, &ev);
				if (event_handler[ev.type])
					event_handler[ev.type](wm, &ev);
			}
		} else {
			/* bar timer event */

			update_bars(wm);

			tv.tv_sec = BAR_UPDATE_RATE;
			tv.tv_usec = 0;
		}
	}

	return 0;
}

struct wm *
init(const char *cmd)
{
	struct wm *wm = xcalloc(1, sizeof(struct wm));

	if (!(wm->dpy = XOpenDisplay(NULL)))
		die("couldn't open display '%s'\n", getenv("DISPLAY"));

	wm->screen = DefaultScreen(wm->dpy);
	wm->root = RootWindow(wm->dpy, wm->screen);

	/* check if another wm is running */
	checkotherwm(wm);

	wm->width = DisplayWidth(wm->dpy, wm->screen);
	wm->height = DisplayHeight(wm->dpy, wm->screen);
	wm->cmd = cmd;
	wm->keys = settings()->keys;
	wm->motion.type = NoMotion;

	sysinfo_init();

	colors_init(settings()->colors, wm->dpy, wm->screen);
	bars_init(wm->dpy, wm->root, wm->screen, settings()->barfont);

	/* select events to handle */
	XSelectInput(wm->dpy, wm->root, WM_EVENT_MASK);

	/* setup cursors */
	cursors_init(wm->dpy);
	cursor_set(wm->root, NormalCursor, wm->dpy);

	/* setup key bindings */
	key_init(wm->dpy);
	key_grab_all(wm->keys, wm->dpy, wm->root);

	/* init atoms and ewmh */
	atoms_init(wm->dpy);
	ewmh_init(wm->dpy, wm->root);
	ewmh_root_set_name(wm->dpy, wm->root, WMNAME);

	/* create monitors */
	create_monitors(wm);

	/* init ewmh desktop functionality */
	ewmh_root_set_number_of_desktops(wm->dpy, wm->root,
			monitor_count(wm->mons) * N_WORKSPACES);
	ewmh_root_set_current_desktop(wm->dpy, wm->root, 0);

	get_windows(wm);

	set_environment(wm);

	return wm;
}

void
get_windows(struct wm *wm)
{
	XWindowAttributes attr;
	Window d1, d2;
	Window *wins;
	unsigned int i, n;

	if (XQueryTree(wm->dpy, wm->root, &d1, &d2, &wins, &n)) {
		/* normal windows */
		for (i = 0; i < n; i++) {
			if (manageable_window(wm->dpy, wins[i], &attr, 0))
				create_client(wm, wins[i], &attr);
		}
		/* transient windows */
		for (i = 0; i < n; i++) {
			if (manageable_window(wm->dpy, wins[i], &attr, 1))
				create_client(wm, wins[i], &attr);
		}

		if (wins)
			XFree(wins);
	}
}

void
handler_buttonpress(struct wm *wm, XEvent *ev)
{
	XButtonPressedEvent *bpev = &ev->xbutton;
	struct client *c;

	dbg_print(wm, __func__);

	if (wm->motion.type != NoMotion)
		return;

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

void
handler_buttonrelease(struct wm *wm, XEvent *ev)
{
	(void)ev;

	dbg_print(wm, __func__);

	if (wm->motion.start.button == ev->xbutton.button) {
		wm->motion.type = NoMotion;
		XUngrabPointer(wm->dpy, CurrentTime);
	}
}

void
handler_clientmessage(struct wm *wm, XEvent *ev)
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
			DBG("%s: WMState\n", __func__);
		} else if (cmev->message_type == netatom(NetActiveWindow)) {
			/* switch to the given client's monitor and workspace
			 * and set focus on the client when a NetActive event
			 * occurs. the spec is quite ambiguous about this. this
			 * behaviour might be changed */
			set_monitor(wm, c->mon);
			monitor_select_client(c->mon, c, wm->dpy, wm->root, 1);
		} else if (cmev->message_type == atom(WMChangeState)) {
			if (cmev->data.l[0] == IconicState &&
					cmev->format == 32) {
				printf("%s: minimize window\n", __func__);
				/* TODO: fix minimizing */
			}
		} else if (cmev->message_type == netatom(NetDesktop)) {
			unsigned long ws;
			DBG("%s: NetDesktop\n", __func__);
			/* TODO: handle multiple monitors */
			if ((ws = cmev->data.l[0]) != ALL_WS)
				ws = ws % N_WORKSPACES;
			client_set_ws(c, wm->dpy, ws);
			monitor_focus(c->mon, NULL, wm->dpy, wm->root);
			monitor_arrange(c->mon, wm->dpy);
		}
	}
}

void
handler_configurerequest(struct wm *wm, XEvent *ev)
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
		DBG("%s: send XConfigureWindow\n", __func__);
	}

	XSync(wm->dpy, False);
}

void
handler_configurenotify(struct wm *wm, XEvent *ev)
{
	dbg_print(wm, __func__);

	if (ev->xconfigure.window == wm->root) {
		/* restart the WM to make sure that any changes to the
		 * X server are handled (e.g. resolution change etc.) */
		restart(wm);
	}
}

void
handler_destroynotify(struct wm *wm, XEvent *ev)
{
	struct client *c;

	dbg_print(wm, __func__);

	if ((c = find_client_by_window(wm->mons, ev->xdestroywindow.window)))
		remove_client(wm, c, 1);
}

void
handler_enternotify(struct wm *wm, XEvent *ev)
{
	XCrossingEvent *cev = &ev->xcrossing;
	struct client *c;

	dbg_print(wm, __func__);

	if (!(c = find_client_by_window(wm->mons, cev->window)))
		return;

	monitor_focus(c->mon, c, wm->dpy, wm->root);
}

void
handler_expose(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	dbg_print(wm, __func__);
}

void
handler_focusin(struct wm *wm, XEvent *ev)
{
	XFocusChangeEvent *fcev = &ev->xfocus;

	dbg_print(wm, __func__);

	/* reacquire focus from a broken client */
	if(wm->selmon->sel && fcev->window != wm->selmon->sel->win)
		monitor_focus(wm->selmon, wm->selmon->sel, wm->dpy, wm->root);
}

void
handler_keypress(struct wm *wm, XEvent *ev)
{
	XKeyEvent *kev = &ev->xkey;
	struct key *key;

	dbg_print(wm, __func__);

	for (key = wm->keys; key; key = key->next) {
		if (key_pressed(key, wm->dpy, kev->keycode, kev->state)) {
			if (key->action >= 0 && key->action < LASTAction &&
					key_handler[key->action]) {
				key_handler[key->action](wm, key);
				break; /* skip further bindings */
			} else {
				die("unhandled key action (%d), fix this!\n",
						key->action);
			}
		}
	}
}

void
handler_mappingnotify(struct wm *wm, XEvent *ev)
{
	(void)wm;
	(void)ev;
	dbg_print(wm, __func__);
}

void
handler_maprequest(struct wm *wm, XEvent *ev)
{
	static XWindowAttributes attr;
	XMapRequestEvent *mrev = &ev->xmaprequest;

	dbg_print(wm, __func__);

	if(!XGetWindowAttributes(wm->dpy, mrev->window, &attr) ||
			attr.override_redirect)
		return;
	if(!find_client_by_window(wm->mons, mrev->window))
		create_client(wm, mrev->window, &attr);
}

void
handler_motionnotify(struct wm *wm, XEvent *ev)
{
	XMotionEvent *mev = &ev->xmotion;
	struct client *c;

	if ((c = find_client_by_window(wm->mons, mev->window))) {
		int xdiff, ydiff;
		int x, y, w, h;

		x = c->x;
		y = c->y;
		w = c->w;
		h = c->h;

		while(XCheckTypedEvent(wm->dpy, MotionNotify, ev));

		xdiff = mev->x_root - wm->motion.start.x_root;
		ydiff = mev->y_root - wm->motion.start.y_root;

		if (wm->motion.type == ResizeMotion) {
			if (!c->floating) { /* TODO: check if the monitor is arranged */
				w = c->ow;
				h = c->oh;
			} else {
				w = wm->motion.attr.width + xdiff;
				h = wm->motion.attr.height + ydiff;
			}
		} else if (wm->motion.type == MovementMotion) {
			x = wm->motion.attr.x + xdiff;
			y = wm->motion.attr.y + ydiff;
		}

		client_move_resize(c, wm->dpy, x, y, w, h);
		monitor_float_selected(wm->selmon, wm->dpy, 1);
	} else {
		struct monitor *mon = find_monitor_by_pos(wm->mons,
				mev->x_root, mev->y_root);

		set_monitor(wm, mon);

		return;
	}
}

void
handler_propertynotify(struct wm *wm, XEvent *ev)
{
	XPropertyEvent *pev = &ev->xproperty;

	dbg_print(wm, __func__);

	if (pev->window == wm->root)
		handler_propertynotify_root(wm, pev);
	else
		handler_propertynotify_client(wm, pev);

}

void
handler_propertynotify_client(struct wm *wm, XPropertyEvent *ev)
{
	struct client *c;

	if (!(c = find_client_by_window(wm->mons, ev->window)))
		return;

	if (ev->atom == XA_WM_NAME || ev->atom == netatom(NetWMName)) {
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
		client_update_wm_hints(c, wm->dpy, c == wm->selmon->sel);
		DBG("hints\n");
	} else if (ev->atom == netatom(NetWMWindowType)) {
		/* TODO */
		client_update_window_type(c, wm->dpy);
		DBG("window type\n");
	}
}

void
handler_propertynotify_root(struct wm *wm, XPropertyEvent *ev)
{
	(void)wm;

	if (ev->atom == XA_WM_NAME) {
		/* TODO: Two options:
		 * 1.) Read the prop and set the status text and redraw all
		 * bars.
		 * 2.) Handle this in bar_update_loop() or similar instead,
		 * where a program is spawned. That would make it easier to use
		 * the program without fixing a script in .xinitrc etc. */
	}
}

void
handler_unmapnotify(struct wm *wm, XEvent *ev)
{
	struct client *c;
	XUnmapEvent *uev = &ev->xunmap;

	dbg_print(wm, __func__);

	if((c = find_client_by_window(wm->mons, uev->window))) {
		if(uev->send_event)
			client_set_state(c, wm->dpy, WithdrawnState);
		else
			remove_client(wm, c, 0);
	}
}

void
key_handler_kill(struct wm *wm, struct key *key)
{
	(void)key;
	if (wm->selmon->sel)
		client_kill(wm->selmon->sel, wm->dpy);
}

void
key_handler_movewindow(struct wm *wm, struct key *key)
{
	if (key->args && wm->selmon->sel) {
		int ws = atoi(key->args) - 1; /* off-by-one in binding */

		if (VALID_WORKSPACE(ws) && wm->selmon->sel->ws != ALL_WS) {
			client_set_ws(wm->selmon->sel, wm->dpy, ws);
			monitor_focus(wm->selmon, NULL, wm->dpy, wm->root);
			monitor_arrange(wm->selmon, wm->dpy);
		}
	}
}

void
key_handler_quit(struct wm *wm, struct key *key)
{
	(void)key;
	quit(wm, "received exit key command");
}

void
key_handler_restart(struct wm *wm, struct key *key)
{
	(void)key;
	restart(wm);
}

void
key_handler_select(struct wm *wm, struct key *key)
{
	if (key->args) {
		if (STREQ(key->args, "next")) {
			monitor_select_next_client(wm->selmon,
					wm->dpy, wm->root);
		} else if (STREQ(key->args, "prev")) {
			monitor_select_prev_client(wm->selmon,
					wm->dpy, wm->root);
		} else {
			error("%s: invalid arg: '%s'", __func__, key->args);
		}

	}
}

void
key_handler_setlayout(struct wm *wm, struct key *key)
{
	int layout = wm->selmon->ws[wm->selmon->selws].layout;

	if (key->args) {
		if (STREQ(key->args, "horz")) {
			layout = TileHorzLayout;
		} else if (STREQ(key->args, "vert")) {
			layout = TileVertLayout;
		} else if (STREQ(key->args, "matrix")) {
			layout = MatrixLayout;
		} else if (STREQ(key->args, "max")) {
			layout = MaxLayout;
		} else if (STREQ(key->args, "float")) {
			layout = FloatingLayout;
		} else if (STREQ(key->args, "prev")) {
			layout = (layout > 0) ? layout - 1 : LASTLayout - 1;
		} else if (STREQ(key->args, "next")) {
			layout = (layout < LASTLayout - 1) ? layout + 1 : 0;
		}
	}

	monitor_set_layout(wm->selmon, wm->dpy, layout);
}

void
key_handler_setmaster(struct wm *wm, struct key *key)
{
	(void)key;

	monitor_selected_to_master(wm->selmon);
	monitor_arrange(wm->selmon, wm->dpy);
}

void
key_handler_setmfact(struct wm *wm, struct key *key)
{
	struct monitor *mon = wm->selmon;

	if (key->args) {
		if (STREQ(key->args, "+")) {
			mon->ws[mon->selws].mfact = MIN(0.99,
					mon->ws[mon->selws].mfact + M_FACTSTEP);
		} else if (STREQ(key->args, "-")) {
			mon->ws[mon->selws].mfact = MAX(0.01,
					mon->ws[mon->selws].mfact - M_FACTSTEP);
		}

		monitor_arrange(wm->selmon, wm->dpy);
	}
}

void
key_handler_setmnum(struct wm *wm, struct key *key)
{
	struct monitor *mon = wm->selmon;

	if (key->args) {
		if (STREQ(key->args, "+")) {
			mon->ws[mon->selws].nmaster++;
		} else if (STREQ(key->args, "-")) {
			mon->ws[mon->selws].nmaster = MAX(1,
					mon->ws[mon->selws].nmaster - 1);
		}

		monitor_arrange(wm->selmon, wm->dpy);
	}
}

void
key_handler_setws(struct wm *wm, struct key *key)
{
	if (key->args) {
		int ws = atoi(key->args) - 1; /* off-by-one in binding */

		if (ws >= MIN_WS && ws <= MAX_WS)
			monitor_set_ws(wm->selmon, wm->dpy, wm->root, ws);
	}
}

void
key_handler_spawn(struct wm *wm, struct key *key)
{
	(void)wm;
	spawn(key->args);
}

void
key_handler_swap(struct wm *wm, struct key *key)
{
	(void)wm;

	if (key->args) {
		if (STREQ(key->args, "next")) {
			/* TODO: monitor_swap_next(wm->selmon); */
		} else if (STREQ(key->args, "prev")) {
			/* TODO: monitor_swap_prev(wm->selmon); */
		}
	}
}

void
key_handler_togglebar(struct wm *wm, struct key *key)
{
	(void)key;
	monitor_toggle_bar(wm->selmon, wm->dpy);
}

void
key_handler_togglefloat(struct wm *wm, struct key *key)
{
	(void)key;
	if (wm->selmon->sel)
		monitor_float_selected(wm->selmon, wm->dpy,
				!wm->selmon->sel->floating);
}

void
key_handler_togglefs(struct wm *wm, struct key *key)
{
	int fs;

	(void)key;

	if (wm->selmon->sel) {
		fs = !wm->selmon->sel->fullscreen;
		client_set_fullscreen(wm->selmon->sel, wm->dpy, fs);

		if (!fs)
			monitor_arrange(wm->selmon, wm->dpy);
	}
}

int
manageable_window(Display *dpy, Window win, XWindowAttributes *attr,
		int trans)
{
	Window dummy;

	if (!XGetWindowAttributes(dpy, win, attr))
		return 0;
	if (trans && !XGetTransientForHint(dpy, win, &dummy))
		return 0;
	if (!trans && (attr->override_redirect ||
				XGetTransientForHint(dpy, win, &dummy)))
		return 0;
	return (attr->map_state == IsViewable ||
			get_state(dpy, win) == IconicState);
}

void
quit(struct wm *wm, const char *reason)
{
	destroy(wm);
	if (reason)
		die("quitting (%s)\n", reason);
	die("quitting\n");
}

void
remove_client(struct wm *wm, struct client *c, int destroyed)
{
	struct monitor *mon = c->mon;

	monitor_remove_client(mon, c);

	if (!destroyed)
		client_unmap(c, wm->dpy);

	client_free(c);

	monitor_focus(mon, NULL, wm->dpy, wm->root);
	monitor_arrange(mon, wm->dpy);

	update_net_client_list(wm);
}

void
restart(struct wm *wm)
{
	if (wm->cmd) {
		char *cmd = xstrdup(wm->cmd);

		DBG("%s. restarting!\n", __func__);
		destroy(wm);
		execlp("/bin/sh", "sh" , "-c", cmd, NULL);
	}
}

void
set_environment(struct wm *wm)
{
	(void)wm;
	setenv("BAR_FONT", settings()->barfont, 1);
	setenv("BAR_NORM_FG", settings()->colors[BarNormFG], 1);
	setenv("BAR_NORM_BG", settings()->colors[BarNormBG], 1);
	setenv("BAR_SEL_FG", settings()->colors[BarSelFG], 1);
	setenv("BAR_SEL_BG", settings()->colors[BarSelBG], 1);
}

void
set_monitor(struct wm *wm, struct monitor *mon)
{
	if (wm->selmon != mon) {
		monitor_unfocus_selected(wm->selmon, wm->dpy, wm->root);
		wm->selmon = mon;
		monitor_focus(mon, mon->sel, wm->dpy, wm->root);

		ewmh_root_set_current_desktop(wm->dpy, wm->root,
				mon->num * N_WORKSPACES + mon->selws);
	}
}

void
update_bars(struct wm *wm)
{
	/* TODO: test code, to be removed */
	struct monitor *mon;

	sysinfo_update();

	for (mon = wm->mons; mon; mon = mon->next)
		monitor_draw_bar(mon, wm->dpy);

	dbg_print(wm, __func__);
}

void
update_net_client_list(struct wm *wm)
{
	struct monitor *mon;
	struct client *c;

	ewmh_root_client_list_clear(wm->dpy, wm->root);

	for (mon = wm->mons; mon; mon = mon->next)
		for (c = mon->clients; c; c = c->next)
			ewmh_root_client_list_add(wm->dpy, wm->root, c->win);
}

int
xerror(Display *dpy, XErrorEvent *ee)
{
	error("fatal error: request code=%d, error code=%d\n",
			ee->request_code, ee->error_code);
	return 0; /* FIXME: handle/ignore errors in a proper way */
	return xerrxlib(dpy, ee); /* may call exit */
}

int
xerror_checkotherwm(Display *dpy, XErrorEvent *ee)
{
	(void)dpy;
	(void)ee;
	die("another window manager is running\n");
	return 0;
}
