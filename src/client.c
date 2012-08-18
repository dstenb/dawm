#include "dawm.h"

/* Mask macros */
#define BUTTON_MASK (ButtonPressMask | ButtonReleaseMask)
#define BUTTON_MOD MOD_SUPER
#define EVENT_MASK (EnterWindowMask | FocusChangeMask | \
	PropertyChangeMask | StructureNotifyMask)
#define MOVE_RESIZE_MASK (CWX | CWY | CWWidth | CWHeight)

bool client_apply_hints(struct client *, int *, int *, int *, int *, bool);
void client_grab_buttons(struct client *);
void client_reset_size_hints(struct client *, bool);
void client_select_input(struct client *);
void client_set_border(struct client *, int);
void client_set_dialog_wtype(struct client *);
void client_set_dock_wtype(struct client *);

bool
client_apply_hints(struct client *c, int *x, int *y,
		int *w, int *h, bool interact)
{
	bool baseismin;
	struct size_hints *hints = c->shints;
	struct monitor *m = c->mon;

	/* Set minimum possible */
	*w = MAX(1, *w);
	*h = MAX(1, *h);

	if(interact) {
		if(*x > screen_w)
			*x = screen_w - WIDTH(c);
		if(*y > screen_h)
			*y = screen_h - HEIGHT(c);
		if(*x + *w + 2 * c->bw < 0)
			*x = 0;
		if(*y + *h + 2 * c->bw < 0)
			*y = 0;
	} else {
		if(*x >= m->wx + m->ww)
			*x = m->wx + m->ww - WIDTH(c);
		if(*y >= m->wy + m->wh)
			*y = m->wy + m->wh - HEIGHT(c);
		if(*x + *w + 2 * c->bw <= m->wx)
			*x = m->wx;
		if(*y + *h + 2 * c->bw <= m->wy)
			*y = m->wy;
	}

	if(hints->honor || c->floating) {
		/* See last two sentences in ICCCM 4.1.2.3 */
		baseismin = hints->basew == hints->minw &&
			hints->baseh == hints->minh;

		if(!baseismin) { /* Temporarily remove base dimensions */
			*w -= hints->basew;
			*h -= hints->baseh;
		}

		/* Adjust for aspect limits */
		if(hints->mina > 0 && hints->maxa > 0) {
			if(hints->maxa < (float)*w / *h)
				*w = *h * hints->maxa + 0.5;
			else if(hints->mina < (float)*h / *w)
				*h = *w * hints->mina + 0.5;
		}

		if(baseismin) { /* Increment calculation requires this */
			*w -= hints->basew;
			*h -= hints->baseh;
		}

		/* Adjust for increment value */
		if(hints->incw)
			*w -= *w % hints->incw;
		if(hints->inch)
			*h -= *h % hints->inch;

		/* Restore base dimensions */
		*w = MAX(*w + hints->basew, hints->minw);
		*h = MAX(*h + hints->baseh, hints->minh);

		if(hints->maxw)
			*w = MIN(*w, hints->maxw);
		if(hints->maxh)
			*h = MIN(*h, hints->maxh);
	}

	return *x != c->x || *y != c->y || *w != c->w || *h != c->h;

}

/** Create a client */
struct client *
client_create(Window win, XWindowAttributes *wa)
{
	struct client *c = xcalloc(1, sizeof(struct client));

	c->x = c->ox = wa->x;
	c->y = c->oy = wa->y;
	c->w = c->ow = wa->width;
	c->h = c->oh = wa->height;
	c->obw = wa->border_width;

	c->name[0] = '\0';
	c->floating = false;
	c->fullscreen = false;
	c->ostate = 0;
	c->wtype = Normal;
	c->prev = NULL;
	c->next = NULL;
	c->snext = NULL;
	c->win = win;

	c->shints = xcalloc(1, sizeof(struct size_hints));
	c->strut = xcalloc(1, sizeof(struct strut_data));

	return c;
}

/** Free the client */
void
client_free(struct client *c)
{
	free(c->shints);
	free(c->strut);
	free(c);
}

/** Grab buttons */
void
client_grab_buttons(struct client *c)
{
	XGrabButton(dpy, L_BUTTON, BUTTON_MOD, c->win, True, ButtonPressMask,
			GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, R_BUTTON, BUTTON_MOD, c->win, True, ButtonPressMask,
			GrabModeAsync, GrabModeAsync, None, None);
}

/** Kill the client. The removal and cleanup is handled in
 * handler_destroynotify() */
void
client_kill(struct client *c)
{
	int (*xerror) (Display *, XErrorEvent *);

	if (!send_event(c->win, atom(WMDelete))) {
		XGrabServer(dpy);

		xerror = XSetErrorHandler(xerror_dummy);

		XSetCloseDownMode(dpy, DestroyAll);
		XKillClient(dpy, c->win);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}
}

/** Maps the window */
void
client_map_window(struct client *c)
{
	XMapWindow(dpy, c->win);
}

/** Move and resize the client */
void
client_move_resize(struct client *c,
		int x, int y, int w, int h, bool hints, bool interact)
{
	XWindowChanges wc;

	if (!hints || client_apply_hints(c, &x, &y, &w, &h, interact)) {
		c->x = wc.x = x;
		c->y = wc.y = y;
		c->w = wc.width = MAX(1, w);
		c->h = wc.height = MAX(1, h);

		XConfigureWindow(dpy, c->win, MOVE_RESIZE_MASK, &wc);
		XSync(dpy, False);
	}
}

/** Raise the client */
void
client_raise(struct client *c)
{
	XRaiseWindow(dpy, c->win);
}

void
client_reset_size_hints(struct client *c, bool honor)
{
	struct size_hints *h = c->shints;

	h->basew = h->baseh = 0;
	h->incw = h->inch = 0;
	h->maxw = h->maxh = 0;
	h->minw = h->minh = 0;
	h->mina = h->maxa = 0.0;
	h->fixed = false;
	h->honor = honor;
}

/** Set the client's events to listen for */
void
client_select_input(struct client *c)
{
	XSelectInput(dpy, c->win, EVENT_MASK);
}

/** Set the border size and color */
void
client_set_border(struct client *c, int bw)
{
	XWindowChanges wc;
	ColorID cid = WinNormBorder;

	if (c->mon && (c->mon->sel == c) && !c->neverfocus)
		cid = WinSelBorder;

	wc.border_width = c->bw = bw;
	XConfigureWindow(dpy, c->win, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, c->win, color(cid));
}

/**  */
void
client_set_dialog_wtype(struct client *c)
{
	c->floating = true;
	c->wtype |= Dialog;
}

/**  */
void
client_set_dock_wtype(struct client *c)
{
	if (ewmh_client_get_strut_partial(c->win, c->strut) ||
			ewmh_client_get_strut(c->win, c->strut)) {
		c->floating = true;
		c->neverfocus = true;
		c->wtype |= Dock;
	}
}

/**  */
void
client_set_floating(struct client *c, bool floating)
{
	if (floating) {
		if (!c->floating)
			client_move_resize(c, c->ox, c->oy, c->ow, c->oh,
					true, false);
		client_raise(c);
		c->floating = true;
	} else {
		/* Save current size */
		if(c->floating) {
			c->ox = c->x;
			c->oy = c->y;
			c->ow = c->w;
			c->oh = c->h;
		}
		c->floating = false;
	}
}

/**  */
void
client_set_focus(struct client *c, bool focus)
{
	if (focus) {
		if (!c->neverfocus) {
			XSetWindowBorder(dpy, c->win, color(WinSelBorder));
			XSetInputFocus(dpy, c->win, RevertToPointerRoot,
					CurrentTime);
		}

		ewmh_root_set_active_window(c->win);
		send_event(c->win, atom(WMTakeFocus));
	} else {
		XSetWindowBorder(dpy, c->win, color(WinNormBorder));
	}
}

/**  */
void
client_set_fullscreen(struct client *c, bool fullscreen)
{
	if (fullscreen && !c->fullscreen) {
		ewmh_client_set_state(c->win, netatom(NetWMFullscreen));

		c->ostate = c->floating;
		c->fullscreen = true;
		c->floating = true;
		c->obw = c->bw;

		c->ox = c->x;
		c->oy = c->y;
		c->ow = c->w;
		c->oh = c->h;

		client_set_border(c, 0);
		client_move_resize(c, c->mon->mx, c->mon->my, c->mon->mw,
				c->mon->mh, false, false);
		client_raise(c);
	} else if (!fullscreen && c->fullscreen) {
		ewmh_client_set_state(c->win, 0);

		c->fullscreen = false;
		c->floating = c->ostate;

		client_set_border(c, c->obw);
		client_move_resize(c, c->ox, c->oy, c->ow, c->oh,
				false, false);
	}
}

/** Set the WM_STATE */
void
client_set_state(struct client *c, long state)
{
	long data[] = { state, None };

	error("%s\n", __func__);
	XChangeProperty(dpy, c->win, atom(WMState), atom(WMState), 32,
			PropModeReplace, (unsigned char *)data, 2);
}

/**  */
void
client_set_ws(struct client *c, unsigned long ws)
{
	assert(VALID_WORKSPACE(ws));

	c->ws = ws;
	ewmh_client_set_desktop(c->win, c->mon->num * N_WORKSPACES + ws);
}

/**  */
void
client_setup(struct client *c, struct monitor *selmon, struct monitor *mons,
		struct client *trans)
{
	unsigned long ws;

	if (trans) {
		c->mon = trans->mon;
		ws = trans->ws;
	} else {
		c->mon = selmon;
		ws = selmon->selws_i;

		/* (try to) get previous desktop value and set the monitor
		 * and workspace according to that */
		if (ewmh_client_get_desktop(c->win, &ws) && ws != ALL_WS) {
			if (find_monitor_by_ws(mons, ws))
				c->mon = find_monitor_by_ws(mons, ws);
			ws = ws % N_WORKSPACES;
		}
	}

	client_reset_size_hints(c, true);
	client_set_ws(c, ws);
	client_set_border(c, settings()->bw);
	client_update_title(c);
	client_update_window_type(c);
	client_update_wm_hints(c, true);
	client_update_size_hints(c);
	client_select_input(c);
	client_grab_buttons(c);

	if (!c->floating)
		c->floating = c->shints->fixed;
	if (c->floating)
		client_raise(c);

	/* add the client to the NetClientList */
	ewmh_root_client_list_add(c->win);
}

/**  */
void
client_show(struct client *c, bool show)
{
	if (show)
		XMoveWindow(dpy, c->win, c->x, c->y);
	else
		XMoveWindow(dpy, c->win, -2 * WIDTH(c), c->y);
}

/** Unmap the client and revert window settings */
void
client_unmap(struct client *c)
{
	int (*xerror) (Display *, XErrorEvent *);
	XWindowChanges wc;

	wc.border_width = c->obw;
	XGrabServer(dpy);
	xerror = XSetErrorHandler(xerror_dummy);
	XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
	XUngrabButton(dpy, AnyButton, AnyModifier, c->win);

	client_set_state(c, WithdrawnState);

	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XUngrabServer(dpy);
}

/** Update the size hints */
void
client_update_size_hints(struct client *c)
{
	XSizeHints size;
	struct size_hints *h = c->shints;
	long msize;

	/* Reset the data */
	client_reset_size_hints(c, h->honor);

	if (!XGetWMNormalHints(dpy, c->win, &size, &msize))
		return;

	if (size.flags & PBaseSize) {
		h->basew = size.base_width;
		h->baseh = size.base_height;
	} else if (size.flags & PMinSize) {
		h->basew = size.min_width;
		h->baseh = size.min_height;
	}

	if (size.flags & PResizeInc) {
		h->incw = size.width_inc;
		h->inch = size.height_inc;
	}

	if (size.flags & PMaxSize) {
		h->maxw = size.max_width;
		h->maxh = size.max_height;
	}

	if (size.flags & PMinSize) {
		h->minw = size.min_width;
		h->minh = size.min_height;
	} else if (size.flags & PBaseSize) {
		h->minw = size.base_width;
		h->minh = size.base_height;
	}

	if (size.flags & PAspect) {
		h->mina = (float) size.min_aspect.y / size.min_aspect.x;
		h->maxa = (float) size.max_aspect.x / size.max_aspect.y;
	}

	h->fixed = (h->maxw && h->minw && h->maxh && h->minh
			&& h->maxw == h->minw && h->maxh == h->minh);
}

/** Update the client title */
void
client_update_title(struct client *c)
{
	if (!atom_get_string(c->win, atom(WMName), c->name, CLIENT_NAME_SIZE))
		snprintf(c->name, CLIENT_NAME_SIZE, "unnamed window");
}

/** Update the window type */
void
client_update_window_type(struct client *c)
{
	Atom state;
	Atom *types;
	unsigned i, n;

	if (ewmh_client_get_state(c->win, &state))
		if (state == netatom(NetWMFullscreen))
			client_set_fullscreen(c, true);

	if (ewmh_client_get_window_types(c->win, &types, &n)) {
		c->wtype = Normal;

		for (i = 0; i < n; i++) {
			if (types[i] == netatom(NetWMWindowTypeDialog))
				client_set_dialog_wtype(c);
			else if (types[i] == netatom(NetWMWindowTypeDock))
				client_set_dock_wtype(c);
		}

		XFree(types);
	}
}

/** Update WM hints */
void
client_update_wm_hints(struct client *c, bool selected)
{
	XWMHints *hints;

	if ((hints = XGetWMHints(dpy, c->win))) {
		if (selected && hints->flags & XUrgencyHint) {
			hints->flags &= ~XUrgencyHint;
			XSetWMHints(dpy, c->win, hints);
		}

		if (hints->flags & InputHint)
			c->neverfocus = !hints->input;

		XFree(hints);
	}
}
