#include "client.h"

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
