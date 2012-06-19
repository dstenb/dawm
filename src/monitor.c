#include "monitor.h"

static void monitor_show_hide(struct monitor *, Display *);
static void monitor_update_window_size(struct monitor *);

static void
add_to_clients(struct monitor *mon, struct client *c)
{
	c->mon = mon;
	c->next = mon->clients;
	mon->clients = c;
}

static void
add_to_stack(struct monitor *mon, struct client *c)
{
	c->snext = mon->cstack;
	mon->cstack = c;
}

static void
remove_from_clients(struct monitor *mon, struct client *c)
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

static void
remove_from_stack(struct monitor *mon, struct client *c)
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

static void
show_hide(struct client *c, Display *dpy)
{
	if (c) {
		if (ISVISIBLE(c)) {
			client_show(c, dpy, 1);
			show_hide(c->snext, dpy);
		} else {
			show_hide(c->snext, dpy);
			client_show(c, dpy, 0);
		}
	}
}

void
monitor_add_client(struct monitor *mon, struct client *c)
{
	add_to_clients(mon, c);
	add_to_stack(mon, c);
}

struct monitor *
monitor_append(struct monitor *mons, struct monitor *new)
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

/* TODO: move all tiling code */
static struct client *
next_tiled(struct client *c)
{
	for ( ; c && (c->floating || !ISVISIBLE(c)); c = c->next) ;
	return c;
}

#define N_MASTER 2
#define M_FACT 0.55

void
arrange_tilehorz(struct monitor *mon, Display *dpy)
{
	struct client *c;
	unsigned int i, n, h, mw, my, ty;

	for (n = 0, c = next_tiled(mon->clients); c;
			c = next_tiled(c->next), n++);

	if (n == 0)
		return;

	if (n > N_MASTER)
		mw = N_MASTER ? mon->ww * M_FACT : 0;
	else
		mw = mon->ww;

	for(i = my = ty = 0, c = next_tiled(mon->clients); c;
			c = next_tiled(c->next), i++) {
		if(i < N_MASTER) {
			h = (mon->wh - my) / (MIN(n, N_MASTER) - i);
			client_move_resize(c, dpy, mon->wx, mon->wy + my,
					mw - (2*c->bw), h - (2*c->bw));
			my += HEIGHT(c);
		}
		else {
			h = (mon->wh - ty) / (n - i);
			client_move_resize(c, dpy, mon->wx + mw, mon->wy + ty,
					mon->ww - mw - (2*c->bw),
					h - (2*c->bw));
			ty += HEIGHT(c);
		}
	}
}

void
arrange_tilevert(struct monitor *mon, Display *dpy)
{

}

void
arrange_matrix(struct monitor *mon, Display *dpy)
{

}

void
monitor_arrange(struct monitor *mon, Display *dpy)
{
	switch(mon->tags[mon->seltag].layout) {
		case TileHorzLayout:
			arrange_tilehorz(mon, dpy);
			break;
		case TileVertLayout:
			arrange_tilevert(mon, dpy);
			break;
		case MatrixLayout:
			arrange_matrix(mon, dpy);
			break;
		case FloatingLayout:
			/* Don't arrange */
			break;
		default:
			break;
	}
}

struct monitor *
monitor_create(struct config *cfg, int x, int y, int w, int h,
		Display *dpy, Window root, int screen)
{
	struct monitor *mon = xcalloc(1, sizeof(struct monitor));
	int i;

	mon->bar = bar_create(cfg->topbar, cfg->showbar, x, y, w, 20,
			dpy, root, screen);
	mon->clients = NULL;
	mon->cstack = NULL;
	mon->sel = NULL;

	mon->next = NULL;

	mon->mx = mon->wx = x;
	mon->my = mon->wy = y;
	mon->mw = mon->ww = w;
	mon->mh = mon->wh = h;

	mon->seltag = MIN_TAG;

	for (i = 0; i < N_TAGS; i++) {
		snprintf(mon->tags[i].name, TAG_NAME_LEN, "%i", (i + 1));
		mon->tags[i].layout = DEFAULT_LAYOUT;
	}

	monitor_show_bar(mon, dpy, mon->bar->showbar);

	return mon;
}

void
monitor_dbg_print(struct monitor *m, const char *str)
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

void
monitor_float_selected(struct monitor *mon, Display *dpy, int f)
{
	int was_floating;

	if (mon->sel) {
		was_floating = mon->sel->floating ? 1 : 0;

		if ((mon->sel->floating = f ? 1 : 0))
			client_raise(mon->sel, dpy);

		/* re-arrange if the state have change */
		if (mon->sel->floating != was_floating)
			monitor_arrange(mon, dpy);
	}
}

void
monitor_focus(struct monitor *mon, struct client *c, Display *dpy,
		Window root)
{
	DBG("%s(%p, %p)\n", __func__, (void *)mon, (void *)c);

	if (!c || !ISVISIBLE(c))
		for (c = mon->cstack; c && !ISVISIBLE(c); c = c->snext);
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

void
monitor_remove_client(struct monitor *mon, struct client *c)
{
	remove_from_clients(mon, c);
	remove_from_stack(mon, c);
}

void
monitor_select_client(struct monitor *mon, struct client *c)
{
	mon->sel = c;
}

void
monitor_set_tag(struct monitor *mon, Display *dpy, Window root, int tag)
{
	assert(tag >= MIN_TAG && tag <= MAX_TAG);

	mon->seltag = tag;
	monitor_show_hide(mon, dpy);
	monitor_arrange(mon, dpy);
	monitor_focus(mon, NULL, dpy, root);
}

void
monitor_show_bar(struct monitor *mon, Display *dpy, int show)
{
	mon->bar->showbar = show ? 1 : 0;
	monitor_update_window_size(mon);
	XMoveResizeWindow(dpy, mon->bar->win, mon->wx, mon->bar->y,
			mon->ww, mon->bar->h);
	monitor_arrange(mon, dpy);
	bar_draw(mon->bar, dpy);
}

void
monitor_toggle_bar(struct monitor *mon, Display *dpy)
{
	monitor_show_bar(mon, dpy, !mon->bar->showbar);
}

void
monitor_unfocus_selected(struct monitor *mon, Display *dpy, Window root)
{
	DBG("%s(%p, %p)\n", __func__, (void *)mon, (void *)mon->sel);

	if (mon->sel)
		client_unfocus(mon->sel, dpy, root);
}

void
monitor_update_window_size(struct monitor *mon)
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

struct client *
find_client_by_window(struct monitor *mon, Window win)
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

struct monitor *
find_monitor_by_pos(struct monitor *mon, int x, int y)
{
	for ( ; mon && !INSIDE(x, y, mon->mx, mon->my, mon->mw, mon->mh);
			mon = mon->next) ;
	return mon;
}

void
monitor_show_hide(struct monitor *mon, Display *dpy)
{
	show_hide(mon->cstack, dpy);
}
