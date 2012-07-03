#include "monitor.h"

static void monitor_restack(struct monitor *, Display *);
static void monitor_show_hide(struct monitor *, Display *);
static void monitor_update_window_size(struct monitor *);

static struct client *
next_tiled(struct client *c)
{
	for ( ; c && (c->floating || !ISVISIBLE(c)); c = c->next) ;
	return c;
}

static int
no_of_tiled_clients(struct client *c)
{
	int n;

	for (n = 0, c = next_tiled(c); c; c = next_tiled(c->next), n++);

	return n;
}

static struct client *
next_visible_client(struct client *curr)
{
	struct monitor *mon = curr->mon;
	struct client *c;

	for (c = curr->next; c && !ISVISIBLE(c); c = c->next) ;

	/* search from the start of the client list */
	if (!c)
		for (c = mon->clients; c && !ISVISIBLE(c); c = c->next) ;

	return c;
}

static struct client *
prev_visible_client(struct client *curr)
{
	struct monitor *mon = curr->mon;
	struct client *c = NULL;
	struct client *t;

	for (t = mon->clients; t != curr; t = t->next) {
		if (ISVISIBLE(t))
			c = t;
	}

	if (!c) {
		for ( ; t; t = t->next) {
			if (ISVISIBLE(t))
				c = t;
		}
	}

	return c;
}

static void
arrange_tilehorz(struct monitor *mon, Display *dpy)
{
	struct client *c;
	unsigned int i, n, h, mw, my, ty;

	float mfact = mon->ws[mon->selws].mfact;
	unsigned int nmaster = mon->ws[mon->selws].nmaster;

	if ((n = no_of_tiled_clients(mon->clients)) == 0)
		return;

	if (n > nmaster)
		mw = nmaster ? mon->ww * mfact : 0;
	else
		mw = mon->ww;

	for(i = my = ty = 0, c = next_tiled(mon->clients); c;
			c = next_tiled(c->next), i++) {
		if(i < nmaster) {
			h = (mon->wh - my) / (MIN(n, nmaster) - i);
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

static void
arrange_tilevert(struct monitor *mon, Display *dpy)
{
	struct client *c;
	unsigned int i, n, w, mh, mx, tx;

	float mfact = mon->ws[mon->selws].mfact;
	unsigned int nmaster = mon->ws[mon->selws].nmaster;

	if ((n = no_of_tiled_clients(mon->clients)) == 0)
		return;

	if (n > nmaster)
		mh = nmaster ? mon->wh *  mfact : 0;
	else
		mh = mon->wh;

	for (i = mx = tx = 0, c = next_tiled(mon->clients); c;
			c = next_tiled(c->next), i++) {
		if (i < nmaster) {
			w = (mon->ww - mx) / (MIN(n, nmaster) - i);
			client_move_resize(c, dpy, mon->wx + mx, mon->wy,
					w - (2*c->bw), mh - (2*c->bw));
			mx += WIDTH(c);
		} else {
			w = (mon->ww - tx) / (n - i);
			client_move_resize(c, dpy, mon->wx + tx, mon->wy + mh,
					w - (2*c->bw), mon->wh - mh - (2*c->bw));
			tx += WIDTH(c);
		}
	}
}

static void
arrange_matrix(struct monitor *mon, Display *dpy)
{
	(void)mon;
	(void)dpy;
}

static void
arrange_max(struct monitor *mon, Display *dpy)
{
	struct client *c;

	for (c = next_tiled(mon->clients); c; c = next_tiled(c->next)) {
		client_move_resize(c, dpy, mon->wx, mon->wy,
				mon->ww - (2*c->bw), mon->wh - (2*c->bw));
	}
}

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

void
monitor_arrange(struct monitor *mon, Display *dpy)
{
	monitor_show_hide(mon, dpy);

	switch(mon->ws[mon->selws].layout) {
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
		case MaxLayout:
			arrange_max(mon, dpy);
			break;
		default:
			break;
	}

	monitor_restack(mon, dpy);
}

int
monitor_count(struct monitor *mon)
{
	int i;

	for (i = 0; mon; mon = mon->next, i++);
	return i;
}

struct monitor *
monitor_create(struct config *cfg, int num, int x, int y, int w, int h,
		Display *dpy, Window root, int screen)
{
	struct monitor *mon = xcalloc(1, sizeof(struct monitor));
	int i;

	mon->bar = bar_create(cfg->topbar, cfg->showbar,
			x, y, w, dpy, root, screen);
	mon->num = num;

	mon->clients = NULL;
	mon->cstack = NULL;
	mon->sel = NULL;

	mon->next = NULL;

	mon->mx = mon->wx = x;
	mon->my = mon->wy = y;
	mon->mw = mon->ww = w;
	mon->mh = mon->wh = h;

	mon->selws = MIN_WS;

	for (i = 0; i < N_WORKSPACES; i++) {
		snprintf(mon->ws[i].name, WS_NAME_SIZE, "%i", (i + 1));
		mon->ws[i].layout = DEFAULT_LAYOUT;
		mon->ws[i].nmaster = cfg->nmaster;
		mon->ws[i].mfact = cfg->mfact;
	}

	monitor_show_bar(mon, dpy, mon->bar->showbar);

	return mon;
}

void
monitor_draw_bar(struct monitor *mon, Display *dpy)
{
	char buf[512];
	char timestr[64];
	const struct info *i = info();

	strftime(timestr, sizeof(timestr), "%y/%m/%d %H:%M",
			localtime(&i->time));

#ifdef __linux__
	char *batstr;

	if (i->bat_status == Charging)
		batstr = "+";
	else if (i->bat_status == Discharging)
		batstr = "-";
	else
		batstr = "";

	snprintf(buf, sizeof(buf), "  %i:%i  %s  UPTIME: %ld:%02ld  CPU: %i%c  "
			"MEM: %ld/%ldMB  BAT: %s%i%c",
			mon->num + 1, mon->selws + 1, timestr,
			i->uptime / 3600, (i->uptime / 60) % 60,
			i->cpu, '%', i->mem_used / 1024, i->mem_total / 1024,
			batstr, i->bat_level, '%');
#else
	snprintf(buf, sizeof(buf), "  %i:%i  %s", mon->num + 1, mon->selws + 1,
			timestr);
#endif

	bar_draw(mon->bar, dpy, buf);
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
	monitor_draw_bar(mon, dpy);
}

void
monitor_remove_client(struct monitor *mon, struct client *c)
{
	remove_from_clients(mon, c);
	remove_from_stack(mon, c);

	if (c == mon->sel)
		mon->sel = mon->clients;
}

#define RES_MASK (CWSibling | CWStackMode)

void
monitor_restack(struct monitor *mon, Display *dpy)
{
	struct client *c;
	XEvent ev;
	XWindowChanges wc;

	monitor_draw_bar(mon, dpy);

	if (!mon->sel)
		return;
	if (mon->sel->floating || !ISARRANGED(mon))
		XRaiseWindow(dpy, mon->sel->win);

	if (ISARRANGED(mon)) {
		wc.stack_mode = Below;
		wc.sibling = mon->bar->win;

		for (c = mon->cstack; c; c = c->snext) {
			if (!c->floating && ISVISIBLE(c)) {
				XConfigureWindow(dpy, c->win, RES_MASK, &wc);
				wc.sibling = c->win;
			}
		}
	}

	XSync(dpy, False);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
monitor_select_client(struct monitor *mon, struct client *c)
{
	mon->sel = c;
}

void
monitor_select_next_client(struct monitor *mon, Display *dpy, Window root)
{
	struct client *c = NULL;

	if (mon->sel)
		c = next_visible_client(mon->sel);

	if (c) {
		monitor_unfocus_selected(mon, dpy, root);
		monitor_select_client(mon, c);
		monitor_focus(mon, c, dpy, root);
		monitor_restack(mon, dpy);
	}
}

void
monitor_select_prev_client(struct monitor *mon, Display *dpy, Window root)
{
	struct client *c = NULL;

	if (mon->sel)
		c = prev_visible_client(mon->sel);

	if (c) {
		monitor_unfocus_selected(mon, dpy, root);
		monitor_select_client(mon, c);
		monitor_focus(mon, c, dpy, root);
		monitor_restack(mon, dpy);
	}
}

void
monitor_selected_to_master(struct monitor *mon)
{
	if (mon->sel && !mon->sel->floating) {
		remove_from_clients(mon, mon->sel);
		add_to_clients(mon, mon->sel);
	}
}

void
monitor_set_layout(struct monitor *mon, Display *dpy, int layout)
{
	assert(layout >= 0 && layout < LASTLayout);

	mon->ws[mon->selws].layout = layout;
	monitor_arrange(mon, dpy);
}

void
monitor_set_ws(struct monitor *mon, Display *dpy, Window root, int ws)
{
	assert(VALID_WORKSPACE(ws));

	mon->selws = ws;

	monitor_focus(mon, NULL,dpy, root);
	monitor_arrange(mon, dpy);

	ewmh_set_current_desktop(dpy, root, mon->num * N_WORKSPACES + ws);
}

void
monitor_show_bar(struct monitor *mon, Display *dpy, int show)
{
	mon->bar->showbar = show ? 1 : 0;
	monitor_update_window_size(mon);
	XMoveResizeWindow(dpy, mon->bar->win, mon->wx, mon->bar->y,
			mon->ww, mon->bar->h);
	monitor_arrange(mon, dpy);
	monitor_draw_bar(mon, dpy);
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
