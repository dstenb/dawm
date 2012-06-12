#include "client.h"

#define BUTTON_MASK (ButtonPressMask | ButtonReleaseMask)

#define EVENT_MASK (EnterWindowMask | FocusChangeMask | \
	PropertyChangeMask | StructureNotifyMask)

#define COL_NORM 0
#define COL_SEL 1

static int xerror_dummy(Display *, XErrorEvent *);

static struct {
	unsigned long fg;
	unsigned long bg;
	unsigned long border;
} colors[2];

struct client *client_create(Window win, XWindowAttributes *wa)
{
	struct client *c = xcalloc(1, sizeof(struct client));

	c->cur_r.x = c->old_r.x = wa->x;
	c->cur_r.y = c->old_r.y = wa->y;
	c->cur_r.w = c->old_r.w = wa->width;
	c->cur_r.h = c->old_r.h = wa->height;

	c->name[0] = '\0';
	c->floating = 0;
	c->fullscreen = 0;
	c->next = NULL;
	c->snext = NULL;
	c->win = win;

	return c;
}

void client_focus(struct client *c, Display *dpy, Window root)
{
	XSetWindowBorder(dpy, c->win, colors[COL_SEL].border);

	XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
	XChangeProperty(dpy, root, atom(NetActiveWindowAtom), XA_WINDOW, 32,
			PropModeReplace, (unsigned char *) &(c->win), 1);

	send_event(dpy, c->win, atom(WMTakeFocusAtom));
}

void client_free(struct client *c)
{
	free(c);
}

void client_grab_buttons(struct client *c, Display *dpy)
{
	XGrabButton(dpy, AnyButton, AnyModifier, c->win, False, BUTTON_MASK,
			GrabModeAsync, GrabModeSync, None, None);
}

int client_is_visible(struct client *c)
{
	/* TODO */
	return 1;
}

void client_map_window(struct client *c, Display *dpy)
{
	XMapWindow(dpy, c->win);
}

void client_move_resize(struct client *c, Display *dpy,
		int x, int y, int w, int h)
{
	XMoveResizeWindow(dpy, c->win, x, y, MAX(1, w), MAX(1, h));
}

void client_raise(struct client *c, Display *dpy)
{
	if (c->floating)
		XRaiseWindow(dpy, c->win);
}

void client_select_input(struct client *c, Display *dpy)
{
	XSelectInput(dpy, c->win, EVENT_MASK);
}

void client_set_border(struct client *c, Display *dpy, int bsize)
{
	XWindowChanges wc;

	c->bsize = bsize;

	wc.border_width = c->bsize;
	XConfigureWindow(dpy, c->win, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, c->win, colors[COL_NORM].border);
}

void client_set_state(struct client *c, Display *dpy, long state)
{
	long data[] = { state, None };

	error("%s\n", __func__);
	XChangeProperty(dpy, c->win, atom(WMStateAtom), atom(WMStateAtom), 32,
			PropModeReplace, (unsigned char *)data, 2);
}

void client_setup(struct client *c, struct config *cfg, struct monitor *mon,
		Display *dpy, Window root, XWindowAttributes *wa)
{
	c->mon = mon;

	client_update_title(c, dpy);

	/* TODO: fix client rules (rule.h) */

	c->old_bsize = wa->border_width;
	client_set_border(c, dpy, cfg->bsize);

	/* TODO: configureevent */
	/*client_fix_window_type(c);*/
	/* TODO: fix size & wm hints */

	client_select_input(c, dpy);
	client_grab_buttons(c, dpy);
	client_raise(c, dpy);

	/* add the client to the NetClientList */
	net_client_list_add(dpy, root, c->win);
}

void client_unfocus(struct client *c, Display *dpy, Window root)
{
	XSetWindowBorder(dpy, c->win, colors[COL_NORM].border);
	/* TODO */
}

void client_unmap(struct client *c, Display *dpy)
{
	int (*xerror) (Display *, XErrorEvent *);
	XWindowChanges wc;

	wc.border_width = c->old_bsize;
	XGrabServer(dpy);
	xerror = XSetErrorHandler(xerror_dummy);
	XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
	XUngrabButton(dpy, AnyButton, AnyModifier, c->win);

	client_set_state(c, dpy, WithdrawnState);

	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XUngrabServer(dpy);
}

void client_update_title(struct client *c, Display *dpy)
{
	if (!(get_text_prop(dpy, c->win, atom(WMNameAtom), c->name,
					CLIENT_NAME_SIZE)))
		snprintf(c->name, CLIENT_NAME_SIZE, "unnamed window");
	error("c->name: %s\n", c->name);
}

void clients_init_colors(struct config *cfg, Display *dpy, int screen)
{
	colors[COL_NORM].fg = get_color(dpy, screen, cfg->col_win_norm[FG]);
	colors[COL_NORM].bg = get_color(dpy, screen, cfg->col_win_norm[BG]);
	colors[COL_NORM].border = get_color(dpy, screen, cfg->col_win_norm[BORDER]);

	colors[COL_SEL].fg = get_color(dpy, screen, cfg->col_win_sel[FG]);
	colors[COL_SEL].bg = get_color(dpy, screen, cfg->col_win_sel[BG]);
	colors[COL_SEL].border = get_color(dpy, screen, cfg->col_win_sel[BORDER]);
}

int xerror_dummy(Display *dpy, XErrorEvent *ev)
{
	(void)dpy;
	(void)ev;
	return 0;
}
