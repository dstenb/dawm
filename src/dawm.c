#include "dawm.h"

#define WM_EVENT_MASK SubstructureRedirectMask | SubstructureNotifyMask \
	| ButtonPressMask | ButtonReleaseMask | PointerMotionMask | \
	EnterWindowMask | LeaveWindowMask | StructureNotifyMask | \
	PropertyChangeMask

static void checkotherwm(void);
static void create_client(Window, XWindowAttributes *);
static void create_monitors(void);
static void get_windows(void);
static void handler_clientmessage_client(XClientMessageEvent *);
static void handler_clientmessage_root(XClientMessageEvent *);
static void handler_configurerequest_resize(struct client *,
		XConfigureRequestEvent *);
static void handler_motionnotify_move(struct client *, XMotionEvent *);
static void handler_motionnotify_resize(struct client *, XMotionEvent *);
static void handler_propertynotify_client(XPropertyEvent *);
static void handler_propertynotify_root(XPropertyEvent *);
static bool manageable_window(Window, XWindowAttributes *, bool);
static void quit(const char *);
static void remove_client(struct client *, bool);
static void restart(void);
static void set_environment(void);
static void set_fullscreen(struct client *, bool);
static void set_monitor(struct monitor *);
static void update_bars(void);
static void update_net_client_list(void);
static int xerror_checkotherwm(Display *, XErrorEvent *);
static int xerror(Display *, XErrorEvent *);

/* X handlers */
static void handler_buttonpress(XEvent *);
static void handler_buttonrelease(XEvent *);
static void handler_clientmessage(XEvent *);
static void handler_configurerequest(XEvent *);
static void handler_configurenotify(XEvent *);
static void handler_destroynotify(XEvent *);
static void handler_enternotify(XEvent *);
static void handler_expose(XEvent *);
static void handler_focusin(XEvent *);
static void handler_keypress(XEvent *);
static void handler_mappingnotify(XEvent *);
static void handler_maprequest(XEvent *);
static void handler_motionnotify(XEvent *);
static void handler_propertynotify(XEvent *);
static void handler_unmapnotify(XEvent *);

/* Key action handlers */
static void key_handler_kill(struct key *);
static void key_handler_movewindow(struct key *);
static void key_handler_quit(struct key *);
static void key_handler_restart(struct key *);
static void key_handler_select(struct key *);
static void key_handler_setlayout(struct key *);
static void key_handler_setmaster(struct key *);
static void key_handler_setmfact(struct key *);
static void key_handler_setmnum(struct key *);
static void key_handler_setws(struct key *);
static void key_handler_spawn(struct key *);
static void key_handler_swap(struct key *);
static void key_handler_togglebar(struct key *);
static void key_handler_togglefloat(struct key *);
static void key_handler_togglefs(struct key *);
static void key_handler_togglews(struct key *);

static int (*xerrxlib) (Display *, XErrorEvent *);
static void (*event_handler[LASTEvent]) (XEvent *) = {
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

static void (*key_handler[LASTAction]) (struct key *) = {
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
	[ToggleFsAction] = key_handler_togglefs,
	[ToggleWsAction] = key_handler_togglews
};

/*** Global variables ***/
static struct monitor *mons = NULL;
static struct monitor *selmon = NULL;
static struct key *keys = NULL;
static struct motion *motion = NULL;
static const char *cmd = NULL;

static void
dbg_print(const char *str)
{
	(void)str;
	DBG(":::: dbg_print(%s)\n", str);
}

void
checkotherwm(void)
{
	xerrxlib = XSetErrorHandler(xerror_checkotherwm);
	/* this causes an error if some other window manager is running */
	XSelectInput(dpy, root, SubstructureRedirectMask);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XSync(dpy, False);
}

void
create_client(Window win, XWindowAttributes *attr)
{
	struct client *c, *tc;

	c = client_create(win, attr);
	tc = find_client_by_trans(mons, win);

	client_setup(c, selmon, mons, tc);
	client_map_window(c);

	rules_apply(c);

	monitor_add_client(c->mon, c);
}

void
create_monitors(void)
{
#ifdef XINERAMA
	struct monitor *mon;
	int i, nmon;

	if (XineramaIsActive(dpy)) {
		XineramaScreenInfo *xsi = XineramaQueryScreens(dpy, &nmon);

		mons = NULL;

		for (i = 0; i < nmon; i++) {
			mon = monitor_create(i, xsi[i].x_org, xsi[i].y_org,
					xsi[i].width, xsi[i].height);
			mons = monitor_append(mons, mon);
		}

		selmon = mons;
		return;
	}
#endif /* XINERAMA */

	selmon = mons = monitor_create(0, 0, 0, screen_w, screen_h);
}

void
destroy(void)
{
	struct monitor *mon;

	for (mon = mons; mon; mon = mon->next) {
		while (mon->cstack)
			remove_client(mon->cstack, false);
	}

	while (mons)
		mons = monitor_free(mons);

	bars_free();
	cursors_free();
	rules_free();

	x11_destroy();
}

void
eventloop(void)
{
	XEvent ev;
	struct timeval tv;
	fd_set fds;
	int n;

	tv.tv_sec = BAR_UPDATE_RATE;
	tv.tv_usec = 0;

	for (;;) {
		FD_ZERO(&fds);
		FD_SET(dpy_fd, &fds);

		if ((n = select(dpy_fd + 1, &fds, NULL, NULL, &tv)) < 0) {
			if (errno == EINTR) /* interrupt during select() */
				continue;
			die("select(): %s\n", strerror(errno));
		} else if (n > 0) {
			while (XPending(dpy)) {
				XNextEvent(dpy, &ev);
				if (event_handler[ev.type])
					event_handler[ev.type](&ev);
			}
		} else {
			/* Bar timer event */
			update_bars();

			tv.tv_sec = BAR_UPDATE_RATE;
			tv.tv_usec = 0;
		}
	}
}

void
init(const char *_cmd)
{
	x11_init();

	checkotherwm();

	cmd = _cmd;

	/* Setup the motion struct */
	motion = xcalloc(1, sizeof(struct motion));
	motion->type = NoMotion;

	sysinfo_init();

	colors_init(settings()->colors);
	bars_init(settings()->barfont);

	/* Select wvents to handle */
	XSelectInput(dpy, root, WM_EVENT_MASK);

	/* Setup cursors */
	cursors_init();
	cursor_set(root, NormalCursor);

	/* Setup key bindings */
	keys = settings()->keys;
	key_init();
	key_grab_all(keys);

	/* Init atoms and EWMH */
	atoms_init();
	ewmh_init();
	ewmh_root_set_name(WMNAME);

	/* Create monitors */
	create_monitors();

	/* Init ewmh desktop functionality */
	ewmh_root_set_number_of_desktops(
			monitor_count(mons) * N_WORKSPACES);
	ewmh_root_set_current_desktop(0);

	get_windows();
	set_environment();
}

void
get_windows(void)
{
	XWindowAttributes attr;
	Window d1, d2;
	Window *wins;
	unsigned int i, n;

	if (XQueryTree(dpy, root, &d1, &d2, &wins, &n)) {
		/* Normal windows */
		for (i = 0; i < n; i++) {
			if (manageable_window(wins[i], &attr, false))
				create_client(wins[i], &attr);
		}
		/* Transient windows */
		for (i = 0; i < n; i++) {
			if (manageable_window(wins[i], &attr, true))
				create_client(wins[i], &attr);
		}

		if (wins)
			XFree(wins);
	}
}

void
handler_buttonpress(XEvent *ev)
{
	XButtonPressedEvent *bpev = &ev->xbutton;
	struct client *c;

	dbg_print(__func__);

	if (motion->type != NoMotion)
		return;

	if (!(c = find_client_by_window(mons, bpev->window)))
		return;

	XGrabPointer(dpy, ev->xbutton.window, True,
			PointerMotionMask | ButtonReleaseMask, GrabModeAsync,
			GrabModeAsync, None, None, CurrentTime);

	/* Set start motion data */
	XGetWindowAttributes(dpy, ev->xbutton.window, &motion->attr);
	motion->start = ev->xbutton;

	/* Set the motion type */
	if (motion->start.button == 1)
		motion->type = MovementMotion;
	else if (motion->start.button == 3)
		motion->type = ResizeMotion;
}

void
handler_buttonrelease(XEvent *ev)
{
	dbg_print(__func__);

	if (motion->start.button == ev->xbutton.button) {
		motion->type = NoMotion;
		XUngrabPointer(dpy, CurrentTime);
	}
}

void
handler_clientmessage(XEvent *ev)
{
	dbg_print(__func__);

	if (ev->xclient.window == root)
		handler_clientmessage_root(&ev->xclient);
	else
		handler_clientmessage_client(&ev->xclient);
}

void
handler_clientmessage_client(XClientMessageEvent *ev)
{
	struct client *c;

	if (!(c = find_client_by_window(mons, ev->window)))
		return;

	if (ev->message_type == atom(WMState)) {
		DBG("%s: WMState\n", __func__);
	} else if (ev->message_type == netatom(NetActiveWindow)) {
		/* switch to the given client's monitor and workspace
		 * and set focus on the client when a NetActive event
		 * occurs. the spec is quite ambiguous about this. this
		 * behaviour might be changed */
		set_monitor(c->mon);
		monitor_select_client(c->mon, c, true);
	} else if (ev->message_type == atom(WMChangeState)) {
		if (ev->data.l[0] == IconicState && ev->format == 32) {
			printf("%s: minimize window\n", __func__);
			/* TODO: fix minimizing */
		}
	} else if (ev->message_type == netatom(NetDesktop)) {
		unsigned long ws;
		DBG("%s: NetDesktop\n", __func__);
		/* TODO: handle multiple monitors */
		if ((ws = ev->data.l[0]) != ALL_WS)
			ws = ws % N_WORKSPACES;
		client_set_ws(c, ws);
		monitor_focus(c->mon, NULL);
		monitor_arrange(c->mon);
	} else if (ev->message_type == netatom(NetWMState)) {
		if (ev->data.l[1] == netatom(NetWMFullscreen) ||
				ev->data.l[2] == netatom(NetWMFullscreen)) {
			bool fs = false;

			if (ev->data.l[0] == NET_WM_STATE_ADD)
				fs = true;
			else if (ev->data.l[0] == NET_WM_STATE_TOGGLE)
				fs = !c->fullscreen;

			set_fullscreen(c, fs);
		}
	}
}

void
handler_clientmessage_root(XClientMessageEvent *ev)
{
	if (ev->format == 32) {
		/* TODO: Maybe handle
		 * net_current_desktop & net_number_of_desktops */
	}
}

void
handler_configurerequest_resize(struct client *c,
		XConfigureRequestEvent *ev)
{
	if (ev->value_mask & CWX)
		c->x = c->mon->mx + ev->x;
	if (ev->value_mask & CWY)
		c->y = c->mon->my + ev->y;
	if (ev->value_mask & CWWidth)
		c->w = ev->width;
	if (ev->value_mask & CWHeight)
		c->h = ev->height;

	if ((c->x + c->w) > c->mon->mx + c->mon->mw && c->floating)
		c->x = c->mon->mx + (c->mon->mw / 2 - WIDTH(c) / 2);
	if ((c->y + c->h) > c->mon->my + c->mon->mh && c->floating)
		c->y = c->mon->my + (c->mon->mh / 2 - HEIGHT(c) / 2);

	if (ISVISIBLE(c))
		client_move_resize(c, c->x, c->y, c->w, c->h);
}

void
handler_configurerequest(XEvent *ev)
{
	XConfigureRequestEvent *crev = &ev->xconfigurerequest;
	struct client *c;

	dbg_print(__func__);

	if ((c = find_client_by_window(mons, crev->window))) {
		if (crev->value_mask & CWBorderWidth) {
			/* TODO: set border size (crev->border_width) */
			DBG("%s: set window border\n", __func__);
		} else {
			if (ISRESIZABLE(c))
				handler_configurerequest_resize(c, crev);
		}
	} else {
		/* TODO: send XConfigureWindow to window */
		DBG("%s: send XConfigureWindow\n", __func__);
	}

	XSync(dpy, False);
}

void
handler_configurenotify(XEvent *ev)
{
	if (ev->xconfigure.window == root) {
		/* Restart the WM to make sure that any changes to the
		 * X server are handled (e.g. resolution change etc.) */
		restart();
	}
}

void
handler_destroynotify(XEvent *ev)
{
	struct client *c;

	dbg_print(__func__);

	if ((c = find_client_by_window(mons, ev->xdestroywindow.window)))
		remove_client(c, true);
}

void
handler_enternotify(XEvent *ev)
{
	XCrossingEvent *cev = &ev->xcrossing;
	struct client *c;

	dbg_print(__func__);

	if (!(c = find_client_by_window(mons, cev->window)))
		return;

	monitor_select_client(c->mon, c, false);
}

void
handler_expose(XEvent *ev)
{
	(void)ev;
	dbg_print(__func__);
}

void
handler_focusin(XEvent *ev)
{
	XFocusChangeEvent *fcev = &ev->xfocus;

	dbg_print(__func__);

	/* reacquire focus from a broken client */
	if(selmon->sel && fcev->window != selmon->sel->win)
		monitor_focus(selmon, selmon->sel);
}

void
handler_keypress(XEvent *ev)
{
	XKeyEvent *kev = &ev->xkey;
	struct key *key;

	dbg_print(__func__);

	for (key = keys; key; key = key->next) {
		if (key_pressed(key, kev->keycode, kev->state)) {
			if (key->action >= 0 && key->action < LASTAction &&
					key_handler[key->action]) {
				key_handler[key->action](key);
				break; /* Skip further bindings */
			} else {
				die("unhandled key action (%d), fix this!\n",
						key->action);
			}
		}
	}
}

void
handler_mappingnotify(XEvent *ev)
{
	(void)ev;
	dbg_print(__func__);
}

void
handler_maprequest(XEvent *ev)
{
	static XWindowAttributes attr;
	XMapRequestEvent *mrev = &ev->xmaprequest;

	dbg_print(__func__);

	if(!XGetWindowAttributes(dpy, mrev->window, &attr) ||
			attr.override_redirect)
		return;
	if(!find_client_by_window(mons, mrev->window))
		create_client(mrev->window, &attr);
}

void
handler_motionnotify(XEvent *ev)
{
	XMotionEvent *mev = &ev->xmotion;
	struct monitor *mon;
	struct client *c;

	if ((c = find_client_by_window(mons, mev->window))) {
		while(XCheckTypedEvent(dpy, MotionNotify, ev));

		if (c->mon->sel != c)
			monitor_select_client(c->mon, c, false);

		if (motion->type == MovementMotion)
			handler_motionnotify_move(c, mev);
		else if (motion->type == ResizeMotion)
			handler_motionnotify_resize(c, mev);
	} else {
		mon = find_monitor_by_pos(mons, mev->x_root, mev->y_root);
		set_monitor(mon);
	}
}

void
handler_motionnotify_move(struct client *c, XMotionEvent *ev)
{
	struct client *t;
	int x, y, pos;

	if (ISTILED(c)) {
		pos = layout_pos_index(c->mon->selws->layout, ev->x_root,
				ev->y_root);

		if (pos >= 0 && (t = find_nth_tiled_client(c->mon, pos)))
			monitor_swap(c->mon, c, t);
	} else if (ISMOVEABLE(c)) {
		x = ev->x_root + motion->attr.x - motion->start.x_root;
		y = ev->y_root + motion->attr.y - motion->start.y_root;

		client_move_resize(c, x, y, c->w, c->h);
		monitor_draw_bar(selmon);
	}
}

void
handler_motionnotify_resize(struct client *c, XMotionEvent *ev)
{
	int w, h;

	if (ISTILED(c)) {
		/* TODO: handle resizing of tiled clients */
	} else if (ISRESIZABLE(c)) {
		w = ev->x_root + motion->attr.width - motion->start.x_root;
		h = ev->y_root + motion->attr.height - motion->start.y_root;

		client_move_resize(c, c->x, c->y, w, h);
		monitor_draw_bar(selmon);
	}
}

void
handler_propertynotify(XEvent *ev)
{
	dbg_print(__func__);

	if (ev->xproperty.window == root)
		handler_propertynotify_root(&ev->xproperty);
	else
		handler_propertynotify_client(&ev->xproperty);
}

void
handler_propertynotify_client(XPropertyEvent *ev)
{
	struct client *c;

	if (!(c = find_client_by_window(mons, ev->window)))
		return;

	if (ev->atom == XA_WM_NAME || ev->atom == netatom(NetWMName)) {
		client_update_title(c);
		/* TODO: redraw the bar */
	} else if (ev->atom == XA_WM_TRANSIENT_FOR) {
		/* TODO */
		DBG("transient\n");
	} else if (ev->atom == XA_WM_NORMAL_HINTS) {
		/* TODO */
		DBG("normal hints\n");
	} else if (ev->atom == XA_WM_HINTS) {
		/* TODO: check for fullscreen and floating */
		client_update_wm_hints(c, c == c->mon->sel);
		DBG("hints\n");
	} else if (ev->atom == netatom(NetWMWindowType)) {
		/* TODO */
		client_update_window_type(c);
		DBG("window type\n");
	}
}

void
handler_propertynotify_root(XPropertyEvent *ev)
{
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
handler_unmapnotify(XEvent *ev)
{
	struct client *c;
	XUnmapEvent *uev = &ev->xunmap;

	dbg_print(__func__);

	if ((c = find_client_by_window(mons, uev->window))) {
		if(uev->send_event)
			client_set_state(c, WithdrawnState);
		else
			remove_client(c, false);
	}
}

void
key_handler_kill(struct key *key)
{
	(void)key;
	if (selmon->sel)
		client_kill(selmon->sel);
}

void
key_handler_movewindow(struct key *key)
{
	int ws;

	if (key->args && selmon->sel) {
		if (STREQ(key->args, "prev"))
			ws = selmon->sel->ws - 1;
		else if (STREQ(key->args, "next"))
			ws = selmon->sel->ws + 1;
		else
			ws = atoi(key->args) - 1; /* Off-by-one in binding */

		if (ws >= MIN_WS && ws <= MAX_WS && selmon->sel->ws != ALL_WS) {
			client_set_ws(selmon->sel, ws);
			monitor_focus(selmon, NULL);
			monitor_arrange(selmon);
		}
	}
}

void
key_handler_quit(struct key *key)
{
	(void)key;
	quit("received exit key command");
}

void
key_handler_restart(struct key *key)
{
	(void)key;
	restart();
}

void
key_handler_select(struct key *key)
{
	if (key->args) {
		if (STREQ(key->args, "next")) {
			monitor_select_next_client(selmon);
		} else if (STREQ(key->args, "prev")) {
			monitor_select_prev_client(selmon);
		} else {
			error("%s: invalid arg: '%s'", __func__, key->args);
		}
	}
}

void
key_handler_setlayout(struct key *key)
{
	struct monitor *mon = selmon;
	int nid, oid;

	oid = mon->selws->layout->id;

	if (key->args) {
		if (STREQ(key->args, "prev"))
			nid = (oid > 0) ? oid - 1 : LASTLayout - 1;
		else if (STREQ(key->args, "next"))
			nid = (oid < LASTLayout - 1) ? oid + 1 : 0;
		else
			nid = layout_str2id(key->args);

		if (nid != -1 && nid != oid)
			monitor_set_layout(mon, nid);
	}
}

void
key_handler_setmaster(struct key *key)
{
	(void)key;

	monitor_selected_to_master(selmon);
	monitor_arrange(selmon);
}

void
key_handler_setmfact(struct key *key)
{
	struct monitor *mon = selmon;
	struct layout *layout = mon->selws->layout;

	if (key->args) {
		if (STREQ(key->args, "+"))
			layout_set_mfact(layout, layout->mfact + M_FACTSTEP);
		else if (STREQ(key->args, "-"))
			layout_set_mfact(layout, layout->mfact - M_FACTSTEP);
		monitor_arrange(selmon);
	}
}

void
key_handler_setmnum(struct key *key)
{
	struct monitor *mon = selmon;
	struct layout *layout = mon->selws->layout;

	if (key->args) {
		if (STREQ(key->args, "+"))
			layout_set_nmaster(layout, layout->nmaster + 1);
		else if (STREQ(key->args, "-") && layout->nmaster > 0)
			layout_set_nmaster(layout, layout->nmaster - 1);
		monitor_arrange(selmon);
	}
}

void
key_handler_setws(struct key *key)
{
	int ws;

	if (key->args) {
		if (STREQ(key->args, "prev"))
			ws = selmon->selws_i - 1;
		else if (STREQ(key->args, "next"))
			ws = selmon->selws_i + 1;
		else
			ws = atoi(key->args) - 1; /* Off-by-one in binding */

		if (ws >= MIN_WS && ws <= MAX_WS)
			monitor_set_ws(selmon, ws);
	}
}

void
key_handler_spawn(struct key *key)
{
	spawn(key->args);
}

void
key_handler_swap(struct key *key)
{
	struct monitor *mon = selmon;

	if (key->args) {
		if (STREQ(key->args, "next"))
			monitor_swap_next_client(mon);
		else if (STREQ(key->args, "prev"))
			monitor_swap_prev_client(mon);
	}
}

void
key_handler_togglebar(struct key *key)
{
	(void)key;
	monitor_toggle_bar(selmon);
}

void
key_handler_togglefloat(struct key *key)
{
	(void)key;
	if (selmon->sel)
		monitor_float_selected(selmon, !selmon->sel->floating);
}

void
key_handler_togglefs(struct key *key)
{
	struct monitor *mon = selmon;
	(void)key;

	if (mon->sel)
		set_fullscreen(mon->sel, !mon->sel->fullscreen);
}

void
key_handler_togglews(struct key *key)
{
	(void)key;
	monitor_set_ws(selmon, selmon->prevws_i);
}

bool
manageable_window(Window win, XWindowAttributes *attr, bool trans)
{
	Window dummy;

	if (!XGetWindowAttributes(dpy, win, attr))
		return false;
	if (trans && !XGetTransientForHint(dpy, win, &dummy))
		return false;
	if (!trans && (attr->override_redirect ||
				XGetTransientForHint(dpy, win, &dummy)))
		return false;
	return (attr->map_state == IsViewable ||
			get_state(win) == IconicState);
}

void
quit(const char *reason)
{
	destroy();
	if (reason)
		die("quitting (%s)\n", reason);
	die("quitting\n");
}

void
remove_client(struct client *c, bool destroyed)
{
	struct monitor *mon = c->mon;

	monitor_remove_client(mon, c);

	if (!destroyed)
		client_unmap(c);

	client_free(c);

	monitor_focus(mon, NULL);
	monitor_arrange(mon);

	update_net_client_list();
}

void
restart(void)
{
	if (cmd) {
		char *_cmd = xstrdup(cmd);

		DBG("%s. restarting!\n", __func__);
		destroy();
		execlp("/bin/sh", "sh" , "-c", _cmd, NULL);
	}
}

void
set_environment(void)
{
	setenv("BAR_FONT", settings()->barfont, 1);
	setenv("BAR_NORM_FG", settings()->colors[BarNormFG], 1);
	setenv("BAR_NORM_BG", settings()->colors[BarNormBG], 1);
	setenv("BAR_SEL_FG", settings()->colors[BarSelFG], 1);
	setenv("BAR_SEL_BG", settings()->colors[BarSelBG], 1);
}

void
set_fullscreen(struct client *c, bool fullscreen)
{
	client_set_fullscreen(c, fullscreen);

	if (!fullscreen)
		monitor_arrange(c->mon);
}

void
set_monitor(struct monitor *mon)
{
	if (selmon != mon) {
		monitor_unfocus_selected(selmon);
		monitor_focus(mon, mon->sel);
		selmon = mon;

		ewmh_root_set_current_desktop(mon->num * N_WORKSPACES
				+ mon->selws_i);
	}
}

void
update_bars(void)
{
	struct monitor *mon;

	sysinfo_update();

	for (mon = mons; mon; mon = mon->next)
		monitor_draw_bar(mon);

	dbg_print(__func__);
}

void
update_net_client_list(void)
{
	struct monitor *mon;
	struct client *c;

	ewmh_root_client_list_clear();

	for (mon = mons; mon; mon = mon->next)
		for (c = mon->clients; c; c = c->next)
			ewmh_root_client_list_add(c->win);
}

int
xerror(Display *_dpy, XErrorEvent *ee)
{
	error("fatal error: request code=%d, error code=%d\n",
			ee->request_code, ee->error_code);
	return 0; /* FIXME: handle/ignore errors in a proper way */
	return xerrxlib(_dpy, ee); /* may call exit */
}

int
xerror_checkotherwm(Display *_dpy, XErrorEvent *ee)
{
	(void)_dpy;
	(void)ee;
	die("another window manager is running\n");
	return 0;
}
