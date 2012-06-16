#include "monitor.h"

static void monitor_update_window_size(struct monitor *);

void monitor_dbg_print(struct monitor *m, const char *str)
{
	struct client *c;
	DBG("monitor_dbg_print (%s)\n", str);
	DBG("monitor_dbg_print (%s)\n", str);
	DBG("m->sel: %p\n", (void *)m->sel);

	DBG("m->clients:\n");
	for (c = m->clients; c; c = c->next)
		DBG("-> %p\n", c);
	DBG("m->cstack:\n");
	for (c = m->cstack; c; c = c->snext)
		DBG("-> %p\n", c);
	DBG("\n");
}

static void add_to_clients(struct monitor *mon, struct client *c)
{
	c->mon = mon;
	c->next = mon->clients;
	mon->clients = c;
}

static void add_to_stack(struct monitor *mon, struct client *c)
{
	c->snext = mon->cstack;
	mon->cstack = c;
}

static void remove_from_clients(struct monitor *mon, struct client *c)
{
	struct client *trav;

	if (c == mon->clients) {
		mon->clients = mon->clients->next;
	} else {
		for (trav = mon->clients; trav->next && trav->next != c;
				trav = trav->next);
		trav->next = trav->next ? trav->next->next : NULL;
	}
}

static void remove_from_stack(struct monitor *mon, struct client *c)
{
	struct client *trav;

	if (c == mon->cstack) {
		mon->cstack = mon->cstack->snext;
	} else {
		for (trav = mon->cstack; trav->snext && trav->snext != c;
				trav = trav->snext);
		trav->snext = trav->snext ? trav->snext->snext : NULL;
	}
}

void monitor_add_client(struct monitor *mon, struct client *c)
{
	add_to_clients(mon, c);
	add_to_stack(mon, c);
}

struct monitor *monitor_append(struct monitor *mons, struct monitor *new)
{
	struct monitor *trav;

	if (mons) {
		for (trav = mons; trav->next; trav = trav->next) ;
		trav->next = new;
		return mons;
	} else {
		return new;
	}
}

void monitor_arrange(struct monitor *mon, Display *dpy)
{
	struct client *c;
	int i = 0;
	int n = 0;

	/* TODO: handle multiple monitors  */

	/* only one window */
	if (mon->sel && mon->cstack->next == NULL) {
		c = mon->sel;

		if (!c->floating)
			client_move_resize(c, dpy, mon->wx + c->bsize,
					mon->wy + c->bsize,
					mon->ww - (c->bsize * 2),
					mon->wh - (c->bsize * 2));
		return;
	}

	/* TODO: this is buggy and dirty, but works OK for testing */

	for (c = mon->clients; c; c = c->next)
		if (c != mon->sel && !c->floating)
			n++;

	for (c = mon->clients; c; c = c->next) {
		if (c->floating)
			continue;
		if (c == mon->sel) {
			client_move_resize(c, dpy, mon->wx + c->bsize,
					mon->wy + c->bsize,
					mon->ww / 2 - (c->bsize * 2),
					mon->wh - (c->bsize * 2));
		} else {
			int sw = mon->wh / (float) n;
			printf("sw: %i\n", sw);
			client_move_resize(c, dpy,
					mon->wx + mon->ww / 2 + c->bsize,
					mon->wy + i * (sw + c->bsize),
					mon->ww / 2 - (c->bsize * 2),
					sw - (c->bsize * 2));
			i++;
		}
	}

}

struct monitor *monitor_create(struct config *cfg, int x, int y, int w, int h,
		Display *dpy, Window root, int screen)
{
	struct monitor *mon = xcalloc(1, sizeof(struct monitor));

	mon->bar = bar_create(1, 1, x, y, w, 20, dpy, root, screen);
	mon->clients = NULL;
	mon->cstack = NULL;
	mon->sel = NULL;

	mon->next = NULL;

	mon->mx = mon->wx = x;
	mon->my = mon->wy = y;
	mon->mw = mon->ww = w;
	mon->mh = mon->wh = h;

	monitor_show_bar(mon, dpy, mon->bar->showbar);

	return mon;
}

void monitor_unfocus_selected(struct monitor *mon, Display *dpy, Window root)
{
	DBG("%s(%p, %p)\n", __func__, (void *)mon, (void *)mon->sel);

	if (mon->sel)
		client_unfocus(mon->sel, dpy, root);
}

void monitor_focus(struct monitor *mon, struct client *c, Display *dpy,
		Window root)
{
	DBG("%s(%p, %p)\n", __func__, (void *)mon, (void *)c);

	if (!c || !client_is_visible(c))
		for (c = mon->cstack; c && !client_is_visible(c); c = c->snext);
	if (mon->sel && mon->sel != c)
		monitor_unfocus_selected(mon, dpy, root);
	if (c) {
		/* TODO: fix support for multiple monitors */

		/* move the window to the front of the client stack */
		remove_from_stack(mon, c);
		add_to_stack(mon, c);

		client_focus(c, dpy, root);
	}

	mon->sel = c;

	/* TODO draw bar */
	bar_draw(mon->bar, dpy);
}

void monitor_remove_client(struct monitor *mon, struct client *c)
{
	remove_from_clients(mon, c);
	remove_from_stack(mon, c);
}

void monitor_select_client(struct monitor *mon, struct client *c)
{
	mon->sel = c;
}

struct client *find_client_by_window(struct monitor *mon, Window win)
{
	struct client *c;

	for ( ; mon; mon = mon->next) {
		for (c = mon->clients; c; c = c->next) {
			if (c->win == win) {
				return c;
			}
		}
	}

	return NULL;
}

void monitor_show_bar(struct monitor *mon, Display *dpy, int show)
{
	mon->bar->showbar = show ? 1 : 0;
	monitor_update_window_size(mon);
	XMoveResizeWindow(dpy, mon->bar->win, mon->wx, mon->bar->y,
			mon->ww, mon->bar->h);
	monitor_arrange(mon, dpy);
	bar_draw(mon->bar, dpy);
}

void monitor_toggle_bar(struct monitor *mon, Display *dpy)
{
	monitor_show_bar(mon, dpy, !mon->bar->showbar);
}

void monitor_update_window_size(struct monitor *mon)
{
	mon->wy = mon->my;
	mon->wh = mon->mh;

	if (mon->bar->showbar) {
		mon->wh -= mon->bar->h;
		mon->bar->y = mon->bar->topbar ? mon->wy : mon->wy + mon->wh;
		mon->wy = mon->bar->topbar ? mon->wy + mon->bar->h : mon->wy;
	} else {
		mon->bar->y = -mon->bar->h;
	}
}
