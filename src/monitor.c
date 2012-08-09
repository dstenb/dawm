#include "monitor.h"

static void monitor_move_clients(struct monitor *, Display *);
static void monitor_restack(struct monitor *, Display *);
static void monitor_show_hide(struct monitor *, Display *);
static void monitor_swap(struct monitor *, struct client *, struct client *);
static void monitor_update_window_size(struct monitor *);

/* Returns the last tiled client */
static struct client *
last_tiled(struct client *curr)
{
	struct client *c = NULL;

	for ( ; curr; curr = curr->next) {
		if (ISTILED(curr))
			c = curr;
	}

	return c;
}

/** Returns the next tiled client */
static struct client *
next_tiled(struct client *c)
{
	for ( ; c && !ISTILED(c); c = c->next) ;
	return c;
}

/** Returns the previous tiled client */
static struct client *
prev_tiled(struct client *c)
{
	for ( ; c && !ISTILED(c); c = c->prev) ;
	return c;
}

/** Returns the number of visible and tiled clients */
static int
no_of_tiled_clients(struct client *c)
{
	int n;

	for (n = 0, c = next_tiled(c); c; c = next_tiled(c->next), n++);

	return n;
}

/** Returns the next selectable client */
static struct client *
next_selectable_client(struct client *curr)
{
	struct monitor *mon = curr->mon;
	struct client *c;

	for (c = curr->next; c && !ISSELECTABLE(c); c = c->next) ;

	/* search from the start of the client list */
	if (!c)
		for (c = mon->clients; c && !ISSELECTABLE(c); c = c->next) ;

	return c;
}

/** Returns the previous selectable client */
static struct client *
prev_selectable_client(struct client *curr)
{
	struct client *c;

	for (c = curr->prev; c; c = c->prev) {
		if (ISSELECTABLE(c))
			return c;
	}

	for (c = curr; c; c = c->next) {
		if (ISSELECTABLE(c))
			curr = c;
	}

	return curr;
}

/** Prepend the given client to the client list */
static void
add_to_clients(struct monitor *mon, struct client *c)
{
	c->mon = mon;
	c->prev = NULL;
	c->next = mon->clients;
	if (mon->clients)
		mon->clients->prev = c;
	mon->clients = c;
}

/** Add the given client to the client stack */
static void
add_to_stack(struct monitor *mon, struct client *c)
{
	c->snext = mon->cstack;
	mon->cstack = c;
}

/** Remove the given client from the client list */
static void
remove_from_clients(struct monitor *mon, struct client *c)
{
	if (c == mon->clients) {
		mon->clients = mon->clients->next;
		if (mon->clients)
			mon->clients->prev = NULL;
	} else {
		if (c->prev)
			c->prev->next = c->next;
		if (c->next)
			c->next->prev = c->prev;
	}
}

/** Remove the given client from the client stack */
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

/** Recursive show/hide client function */
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

/** Add client to a monitor */
void
monitor_add_client(struct monitor *mon, struct client *c,
		Display *dpy, Window root)
{
	add_to_clients(mon, c);
	add_to_stack(mon, c);

	if (ISDOCK(c) && ISVISIBLE(c))
		monitor_update_window_size(mon);

	/* TODO: check switch_to_ws rule         v */
	monitor_select_client(mon, c, dpy, root, 0);
	monitor_arrange(mon, dpy);
}

/** Append a monitor to another monitor list */
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

/** Arrange the clients on the monitor */
void
monitor_arrange(struct monitor *mon, Display *dpy)
{
	monitor_show_hide(mon, dpy);
	monitor_move_clients(mon, dpy);
	monitor_restack(mon, dpy);
}

/** Count number of monitors */
int
monitor_count(struct monitor *mon)
{
	int i;

	for (i = 0; mon; mon = mon->next, i++);
	return i;
}

/** Creates a monitor */
struct monitor *
monitor_create(int num, int x, int y, int w, int h,
		Display *dpy, Window root, int screen)
{
	struct monitor *mon = xcalloc(1, sizeof(struct monitor));
	int i;

	mon->bar = bar_create(settings()->topbar, settings()->showbar,
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

	mon->selws_i = MIN_WS;
	mon->prevws_i = mon->selws_i;
	mon->selws = &mon->ws[mon->selws_i];

	for (i = 0; i < N_WORKSPACES; i++) {
		snprintf(mon->ws[i].name, WS_NAME_SIZE, "%i", (i + 1));
		mon->ws[i].layout = layout_init(DEFAULT_LAYOUT,
				mon->mw, mon->mh, mon->ww, mon->wh,
				settings()->nmaster, settings()->mfact);
	}

	monitor_show_bar(mon, dpy, mon->bar->showbar);

	return mon;
}

/** Draws the text on the bar */
void
monitor_draw_bar(struct monitor *mon, Display *dpy)
{
	char buf[512];
	char timestr[64];
	const struct sysinfo *i = sysinfo();
	const char *layoutstr = layout_symbol(mon->selws->layout);

	strftime(timestr, sizeof(timestr), "%y/%m/%d %H:%M",
			localtime(&i->time));

#ifdef SYSINFO_EXTENDED
	char *batstr;

	if (i->bat_status == Charging)
		batstr = "+";
	else if (i->bat_status == Discharging)
		batstr = "-";
	else
		batstr = "";

	snprintf(buf, sizeof(buf), "'%s'  %i:%lu  [%s]  %s  UPTIME: %ld:%02ld  "
			"CPU: %i%c  MEM: %ld/%ldMB  BAT: %s%i%c", mon->str,
			mon->num + 1, mon->selws_i + 1, layoutstr, timestr,
			i->uptime / 3600, (i->uptime / 60) % 60,
			i->cpu, '%', i->mem_used / 1024, i->mem_total / 1024,
			batstr, i->bat_level, '%');
#else
	snprintf(buf, sizeof(buf), "  %i:%lu  [%s]  %s", mon->num + 1,
			mon->selws + 1, layoutstr, timestr);
#endif /* SYSINFO_EXTENDED */

	bar_draw(mon->bar, dpy, buf);
}

/** Set the floating state for the selected client */
void
monitor_float_selected(struct monitor *mon, Display *dpy, int floating)
{
	if (mon->sel) {
		client_set_floating(mon->sel, dpy, floating);
		monitor_arrange(mon, dpy);
	}
}

/** Focus on the given client. If no client given, the first available
 * client will be focused */
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

		client_set_focus(c, dpy, root, true);
	}

	mon->sel = c;

	/* TODO draw bar */
	monitor_draw_bar(mon, dpy);
}

/** Frees the given monitor */
struct monitor *
monitor_free(struct monitor *mon, Display *dpy)
{
	struct monitor *next = mon->next;

	bar_free(mon->bar, dpy);
	free(mon);

	return next;
}

/** Move the tiled clients to the places given by the layout */
void
monitor_move_clients(struct monitor *mon, Display *dpy)
{
	struct layout *layout = mon->selws->layout;
	struct layout_pos *pos;
	struct client *c;
	unsigned i;
	int x, y, w, h;

	/* TODO */
	layout_set_clients(layout, no_of_tiled_clients(mon->clients));

	for (i = 0, c = next_tiled(mon->clients); c && i < layout->n;
			c = next_tiled(c->next), i++) {
		pos = &layout->pos[i];

		x = mon->wx + pos->x;
		y = mon->wy + pos->y;
		w = pos->w - (2 * c->bw);
		h = pos->h - (2 * c->bw);

		client_move_resize(c, dpy, x, y, w, h);
	}
}

/** Remove the given client from the monitor */
void
monitor_remove_client(struct monitor *mon, struct client *c)
{
	remove_from_clients(mon, c);
	remove_from_stack(mon, c);

	if (c == mon->sel)
		mon->sel = mon->clients;

	monitor_update_window_size(mon);
}

#define RES_MASK (CWSibling | CWStackMode)

/** Restack all of the monitor's clients */
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
		client_raise(mon->sel, dpy);

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

/** Select the given client and fix focus. The function will do nothing if the
 * client's workspace differs from the current and switch_to_ws is zero */
void
monitor_select_client(struct monitor *mon, struct client *c,
		Display *dpy, Window root, int switch_to_ws)
{
	assert(c->mon == mon);

	if (!ISSELECTABLE(c))
		return;

	if (mon->selws_i != c->ws && c->ws != ALL_WS && !switch_to_ws)
		return;

	monitor_unfocus_selected(mon, dpy, root);
	mon->sel = c;

	if (mon->selws_i == c->ws || c->ws == ALL_WS) {
		monitor_focus(mon, c, dpy, root);
		monitor_restack(mon, dpy);
	} else {
		monitor_set_ws(mon, dpy, root, c->ws);
	}
}

/** Selects the next valid client in the client list */
void
monitor_select_next_client(struct monitor *mon, Display *dpy, Window root)
{
	struct client *c = NULL;

	if (mon->sel && (c = next_selectable_client(mon->sel)))
		monitor_select_client(mon, c, dpy, root, 0);
}

/** Selects the prevous valid client in the client list */
void
monitor_select_prev_client(struct monitor *mon, Display *dpy, Window root)
{
	struct client *c = NULL;

	if (mon->sel && (c = prev_selectable_client(mon->sel)))
		monitor_select_client(mon, c, dpy, root, 0);
}

/** Moves the selected client to the master position */
void
monitor_selected_to_master(struct monitor *mon)
{
	if (mon->sel && !mon->sel->floating) {
		remove_from_clients(mon, mon->sel);
		add_to_clients(mon, mon->sel);
	}
}

/** Set the layout for the selected workspace */
void
monitor_set_layout(struct monitor *mon, Display *dpy, int id)
{
	assert(id >= 0 && id < LASTLayout);

	layout_set(mon->selws->layout, id);
	monitor_arrange(mon, dpy);
}

/** Set the current workspace */
void
monitor_set_ws(struct monitor *mon, Display *dpy, Window root,
		unsigned long ws)
{
	assert(VALID_WORKSPACE(ws));

	mon->prevws_i = mon->selws_i;
	mon->selws_i = ws;
	mon->selws = &mon->ws[ws];

	monitor_update_window_size(mon);
	monitor_focus(mon, NULL, dpy, root);
	monitor_arrange(mon, dpy);

	ewmh_root_set_current_desktop(dpy, root, mon->num * N_WORKSPACES + ws);
}

/** Show/hide the bar */
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

/** Show/hide the clients */
void
monitor_show_hide(struct monitor *mon, Display *dpy)
{
	show_hide(mon->cstack, dpy);
}

/** Swap the two clients */
void
monitor_swap(struct monitor *mon, struct client *c1, struct client *c2)
{
	struct client *tmp;

	tmp = c1->next;
	c1->next = c2->next;
	c2->next = tmp;

	if (c1->next)
		c1->next->prev = c1;
	if (c2->next)
		c2->next->prev = c2;

	tmp = c1->prev;
	c1->prev = c2->prev;
	c2->prev = tmp;

	if (c1->prev)
		c1->prev->next = c1;
	else
		mon->clients = c1;

	if (c2->prev)
		c2->prev->next = c2;
	else
		mon->clients = c2;
}

/** Swaps the currently selected client with the next tiled client */
void
monitor_swap_next_client(struct monitor *mon, Display *dpy)
{
	struct client *sel = mon->sel;
	struct client *c = NULL;

	if (!sel || !ISTILED(sel))
		return;

	if (((c = next_tiled(sel->next)) || (c = next_tiled(mon->clients)))
			&& c != sel) {
		monitor_swap(mon, sel, c);
		monitor_arrange(mon, dpy);
	}
}

/** Swaps the currently selected client with the previous tiled client */
void
monitor_swap_prev_client(struct monitor *mon, Display *dpy)
{
	struct client *sel = mon->sel;
	struct client *c = NULL;

	if (!sel || !ISTILED(sel))
		return;

	if (((c = prev_tiled(sel->prev)) || (c = last_tiled(mon->clients)))
			&& c != sel) {
		monitor_swap(mon, sel, c);
		monitor_arrange(mon, dpy);
	}
}

/** Removes focus from the currently selected client */
void
monitor_unfocus_selected(struct monitor *mon, Display *dpy, Window root)
{
	if (mon->sel)
		client_set_focus(mon->sel, dpy, root, false);
}

/** Update the real window size for the given monitor, i.e. the space that
 * isn't covered by any dock */
void
monitor_update_window_size(struct monitor *mon)
{
	struct client *c;
	unsigned left = 0;
	unsigned right = 0;
	unsigned top = 0;
	unsigned bottom = 0;

	/* Calculate dock offsets */
	for (c = mon->clients; c; c = c->next) {
		if (ISDOCK(c) && ISVISIBLE(c)) {
			left = MAX(left, c->strut->left);
			right = MAX(right, c->strut->right);
			top = MAX(top, c->strut->top);
			bottom = MAX(bottom, c->strut->bottom);
		}
	}

	/* Calculate bar offset */
	if (mon->bar->showbar) {
		if (mon->bar->topbar) {
			top = MAX(top, (unsigned)mon->bar->h);
			mon->bar->y = mon->my;
		} else {
			bottom = MAX(bottom, (unsigned)mon->bar->h);
			mon->bar->y = mon->my + mon->mh - mon->bar->h;
		}
	} else {
		mon->bar->y = -mon->bar->h;
	}

	mon->wx = mon->mx + left;
	mon->wy = mon->my + top;
	mon->ww = mon->mw - left - right;
	mon->wh = mon->mh - top - bottom;

	/* Calculate new layout positions */
	layout_set_geom(mon->selws->layout, mon->ww, mon->wh);
}

/** Searches through all the monitors for the client
 * that is transient for the given window */
struct client *
find_client_by_trans(struct monitor *mon, Display *dpy, Window win)
{
	Window trans = None;

	return XGetTransientForHint(dpy, win, &trans) ?
		find_client_by_window(mon, trans) : NULL;
}

/** Searches through all the monitors for the client
 * corresponding to the given window */
struct client *
find_client_by_window(struct monitor *mon, Window win)
{
	struct client *c;

	for ( ; mon; mon = mon->next) {
		for (c = mon->clients; c; c = c->next) {
			if (c->win == win)
				return c;
		}
	}

	return NULL;
}

/** Returns the monitor that corresponds to the given monitor number */
struct monitor *
find_monitor_by_num(struct monitor *mon, int num)
{
	for ( ; mon && mon->num != num; mon = mon->next) ;
	return mon;
}

/** Returns the monitor that corresponds to the given position */
struct monitor *
find_monitor_by_pos(struct monitor *mon, int x, int y)
{
	for ( ; mon && !INSIDE(x, y, mon->mx, mon->my, mon->mw, mon->mh);
			mon = mon->next) ;
	return mon;
}

/** Returns the monitor that corresponds to the given workspace index */
struct monitor *
find_monitor_by_ws(struct monitor *mon, unsigned index)
{
	for ( ; mon && index > MAX_WS; mon = mon->next, index -= N_WORKSPACES);
	return mon;
}
