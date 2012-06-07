#include "client.h"

#define CLIENT_BUTTON_MASK (ButtonPressMask | ButtonReleaseMask)

#define CLIENT_EVENT_MASK EnterWindowMask | FocusChangeMask | \
	PropertyChangeMask | StructureNotifyMask

#define COL_NORM 0
#define COL_SEL 1

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
	c->win = win;

	return c;
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

void client_set_name(struct client *c, const char *str)
{
	strncpy(c->name, str, CLIENT_NAME_SIZE);
	error("%s: %s\n", __func__, c->name);
}

void client_update_title(struct client *c, Display *dpy)
{
	Atom prop = XInternAtom(dpy, "WM_NAME", False);

	get_text_prop(dpy, c->win, prop, c->name, CLIENT_NAME_SIZE);
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
