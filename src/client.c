#include "client.h"

#define CLIENT_BUTTON_MASK (ButtonPressMask | ButtonReleaseMask)

#define CLIENT_EVENT_MASK (EnterWindowMask | FocusChangeMask | \
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
	Atom atom = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);

	XSetWindowBorder(dpy, c->win, colors[COL_SEL].border);

	XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
	XChangeProperty(dpy, root, atom, XA_WINDOW, 32, PropModeReplace,
			(unsigned char *) &(c->win), 1);

	/* TODO: send WM_TAKE_FOCUS event */
}

void client_free(struct client *c)
{
	free(c);
}

void client_grab_buttons(struct client *c, Display *dpy)
{
	XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
			CLIENT_BUTTON_MASK, GrabModeAsync, GrabModeSync, None,
			None);
}

int client_is_visible(struct client *c)
{
	/* TODO */
	return 1;
}

void client_move_resize(struct client *c, Display *dpy,
		int x, int y, int w, int h)
{
	XMoveResizeWindow(dpy, c->win, x, y, w, h);
}

void client_raise(struct client *c, Display *dpy)
{
	if (c->floating)
		XRaiseWindow(dpy, c->win);
}

void client_select_input(struct client *c, Display *dpy)
{
	XSelectInput(dpy, c->win, CLIENT_EVENT_MASK);
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
	Atom atom = XInternAtom(dpy, "WM_STATE", False);
	long data[] = { state, None };

	error("%s\n", __func__);
	XChangeProperty(dpy, c->win, atom, atom, 32, PropModeReplace,
			(unsigned char *)data, 2);
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
	Atom prop = XInternAtom(dpy, "WM_NAME", False);
	get_text_prop(dpy, c->win, prop, c->name, CLIENT_NAME_SIZE);
	error("c->name: %s\n", c->name);
}

void clients_init_colors(struct config *cfg, Display *dpy, int screen)
{
	colors[COL_NORM].fg = get_color(dpy, screen, cfg->col_normfg);
	colors[COL_NORM].bg = get_color(dpy, screen, cfg->col_normbg);
	colors[COL_NORM].border = get_color(dpy, screen, cfg->col_normborder);

	colors[COL_SEL].fg = get_color(dpy, screen, cfg->col_selfg);
	colors[COL_SEL].bg = get_color(dpy, screen, cfg->col_selbg);
	colors[COL_SEL].border = get_color(dpy, screen, cfg->col_selborder);
}

int xerror_dummy(Display *dpy, XErrorEvent *ev)
{
	(void)dpy;
	(void)ev;
	return 0;
}
