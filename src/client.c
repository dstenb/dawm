#include "client.h"

#define BUTTON_MASK (ButtonPressMask | ButtonReleaseMask)

#define BUTTON_MOD MOD_SUPER

#define EVENT_MASK (EnterWindowMask | FocusChangeMask | \
	PropertyChangeMask | StructureNotifyMask)

#define MOVE_RESIZE_MASK (CWX | CWY | CWWidth | CWHeight)

void client_grab_buttons(struct client *, Display *);
void client_select_input(struct client *, Display *);
void client_set_border(struct client *, Display *, int);
static int xerror_dummy(Display *, XErrorEvent *);

/** create a client */
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
	c->floating = 0;
	c->fullscreen = 0;
	c->ostate = 0;
	c->prev = NULL;
	c->next = NULL;
	c->snext = NULL;
	c->win = win;

	return c;
}

/** free the client */
void
client_free(struct client *c)
{
	free(c);
}

/** grab buttons */
void
client_grab_buttons(struct client *c, Display *dpy)
{
	XGrabButton(dpy, 1, BUTTON_MOD, c->win, True, ButtonPressMask,
			GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 3, BUTTON_MOD, c->win, True, ButtonPressMask,
			GrabModeAsync, GrabModeAsync, None, None);
}

/** kill the client. the removal will be handled in wm_handler_destroynotify */
void
client_kill(struct client *c, Display *dpy)
{
	int (*xerror) (Display *, XErrorEvent *);

	if (!send_event(dpy, c->win, atom(WMDelete))) {
		XGrabServer(dpy);

		xerror = XSetErrorHandler(xerror_dummy);

		XSetCloseDownMode(dpy, DestroyAll);
		XKillClient(dpy, c->win);
		XSync(dpy, False);

		XSetErrorHandler(xerror);

		XUngrabServer(dpy);
	}
}

void
client_map_window(struct client *c, Display *dpy)
{
	XMapWindow(dpy, c->win);
}

/** move and resize the client */
void
client_move_resize(struct client *c, Display *dpy,
		int x, int y, int w, int h)
{
	XWindowChanges wc;

	c->x = wc.x = x;
	c->y = wc.y = y;
	c->w = wc.width = MAX(1, w);
	c->h = wc.height = MAX(1, h);

	XConfigureWindow(dpy, c->win, MOVE_RESIZE_MASK, &wc);
	XSync(dpy, False);
}

/** raise the client */
void
client_raise(struct client *c, Display *dpy)
{
	XRaiseWindow(dpy, c->win);
}

/** set the client's events to listen for */
void
client_select_input(struct client *c, Display *dpy)
{
	XSelectInput(dpy, c->win, EVENT_MASK);
}

/** set the border size and color */
void
client_set_border(struct client *c, Display *dpy, int bw)
{
	XWindowChanges wc;
	ColorID cid = WinNormBorder;

	if (c->mon)
		cid = (c == c->mon->sel) ? WinSelBorder : WinNormBorder;

	wc.border_width = c->bw = bw;
	XConfigureWindow(dpy, c->win, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, c->win, color(cid));
}

void
client_set_floating(struct client *c, Display *dpy, int floating)
{
	if (floating) {
		if (!c->floating)
			client_move_resize(c, dpy, c->ox, c->oy, c->ow, c->oh);
		client_raise(c, dpy);
		c->floating = 1;
	} else {
		/* save current size */
		if(c->floating) {
			c->ox = c->x;
			c->oy = c->y;
			c->ow = c->w;
			c->oh = c->h;
		}
		c->floating = 0;
	}
}

void
client_set_focus(struct client *c, Display *dpy, Window root, int focus)
{
	if (focus) {
		XSetWindowBorder(dpy, c->win, color(WinSelBorder));
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);

		ewmh_root_set_active_window(dpy, root, c->win);
		send_event(dpy, c->win, atom(WMTakeFocus));
	} else {
		XSetWindowBorder(dpy, c->win, color(WinNormBorder));
	}
}

void
client_set_fullscreen(struct client *c, Display *dpy, int fullscreen)
{
	if (fullscreen) {
		ewmh_client_set_state(dpy, c->win, netatom(NetWMFullscreen));

		c->fullscreen = 1;
		c->ostate = c->floating;
		c->floating = 1;
		c->obw = c->bw;

		c->ox = c->x;
		c->oy = c->y;
		c->ow = c->w;
		c->oh = c->h;

		client_set_border(c, dpy, 0);
		client_move_resize(c, dpy, c->mon->mx, c->mon->my,
				c->mon->mw, c->mon->mh);
		client_raise(c, dpy);
	} else {
		ewmh_client_set_state(dpy, c->win, 0);

		c->fullscreen = 0;
		c->floating = c->ostate;

		client_set_border(c, dpy, c->obw);
		client_move_resize(c, dpy, c->ox, c->oy, c->ow, c->oh);
	}
}

/** set the WM_STATE */
void
client_set_state(struct client *c, Display *dpy, long state)
{
	long data[] = { state, None };

	error("%s\n", __func__);
	XChangeProperty(dpy, c->win, atom(WMState), atom(WMState), 32,
			PropModeReplace, (unsigned char *)data, 2);
}

void
client_set_ws(struct client *c, Display *dpy, unsigned long ws)
{
	assert(VALID_WORKSPACE(ws));

	c->ws = ws;
	ewmh_client_set_desktop(dpy, c->win, c->mon->num * N_WORKSPACES + ws);
}

void
client_setup(struct client *c, struct config *cfg, struct monitor *selmon,
		struct monitor *mons, Display *dpy, Window root,
		struct client *trans)
{
	unsigned long ws;

	client_update_title(c, dpy);

	if (trans) {
		c->mon = trans->mon;
		ws = trans->ws;
	} else {
		c->mon = selmon;
		ws = selmon->selws;

		/* (try to) get previous desktop value and set the monitor
		 * and workspace according to that */
		if (ewmh_client_get_desktop(dpy, c->win, &ws) && ws != ALL_WS) {
			if (find_monitor_by_ws(mons, ws))
				c->mon = find_monitor_by_ws(mons, ws);
			ws = ws % N_WORKSPACES;
		}
	}

	client_set_ws(c, dpy, ws);
	client_set_border(c, dpy, cfg->bw);
	/* TODO: configureevent */
	client_update_window_type(c, dpy);
	/* TODO: fix size & wm hints */

	/* TODO: rule_apply_all(cfg->rules, c, selmon, mons); */

	client_select_input(c, dpy);
	client_grab_buttons(c, dpy);

	if (c->floating)
		client_raise(c, dpy);

	/* add the client to the NetClientList */
	ewmh_root_client_list_add(dpy, root, c->win);
}

void
client_show(struct client *c, Display *dpy, int show)
{
	if (show)
		XMoveWindow(dpy, c->win, c->x, c->y);
	else
		XMoveWindow(dpy, c->win, -2 * WIDTH(c), c->y);
}

/** unmap the client and revert window settings */
void
client_unmap(struct client *c, Display *dpy)
{
	int (*xerror) (Display *, XErrorEvent *);
	XWindowChanges wc;

	wc.border_width = c->obw;
	XGrabServer(dpy);
	xerror = XSetErrorHandler(xerror_dummy);
	XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
	XUngrabButton(dpy, AnyButton, AnyModifier, c->win);

	client_set_state(c, dpy, WithdrawnState);

	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XUngrabServer(dpy);
}

/** update the client title */
void
client_update_title(struct client *c, Display *dpy)
{
	if (!(atom_get_string(dpy, c->win, atom(WMName), c->name,
					CLIENT_NAME_SIZE)))
		snprintf(c->name, CLIENT_NAME_SIZE, "unnamed window");
	error("c->name: %s\n", c->name);
}


void
client_update_window_type(struct client *c, Display *dpy)
{
	Atom state;
	Atom *types;
	unsigned i, n;

	if (ewmh_client_get_state(dpy, c->win, &state))
		if (state == netatom(NetWMFullscreen))
			client_set_fullscreen(c, dpy, 1);

	if (ewmh_client_get_window_types(dpy, c->win, &types, &n)) {
		for (i = 0; i < n; i++) {
			if (types[i] == netatom(NetWMWindowTypeDialog))
				c->floating = 1;
		}

		XFree(types);
	}
}

int
xerror_dummy(Display *dpy, XErrorEvent *ev)
{
	(void)dpy;
	(void)ev;
	return 0;
}
