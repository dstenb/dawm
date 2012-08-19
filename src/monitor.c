#include "dawm.h"

static void monitor_move_clients(struct monitor *);
static void monitor_restack(struct monitor *);
static void monitor_show_hide(struct monitor *);
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
show_hide(struct client *c)
{
	if (c) {
		if (ISVISIBLE(c)) {
			client_show(c, true);
			show_hide(c->snext);
		} else {
			show_hide(c->snext);
			client_show(c, false);
		}
	}
}

/** Add client to a monitor */
void
monitor_add_client(struct monitor *mon, struct client *c)
{
	add_to_clients(mon, c);
	add_to_stack(mon, c);

	if (ISDOCK(c) && ISVISIBLE(c))
		monitor_update_window_size(mon);

	/* TODO: check switch_to_ws rule         v */
	monitor_select_client(mon, c, false);
	monitor_arrange(mon);
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
monitor_arrange(struct monitor *mon)
{
	monitor_show_hide(mon);
	monitor_move_clients(mon);
	monitor_restack(mon);
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
monitor_create(int num, int x, int y, int w, int h)
{
	struct monitor *mon = xcalloc(1, sizeof(struct monitor));
	const struct ws_settings *wss;
	int i;

	mon->bar = bar_create(settings()->topbar, settings()->showbar, x, y, w);
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
		wss = &settings()->ws[i];

		strcpy(mon->ws[i].name, wss->name);
		mon->ws[i].layout = layout_init(wss->layout, mon->mw, mon->mh,
				mon->ww, mon->wh, wss->nmaster, wss->mfact);
	}

	monitor_show_bar(mon, mon->bar->showbar);

	return mon;
}

/** Draws the text on the bar */
void
monitor_draw_bar(struct monitor *mon)
{
	struct format_data fd = {
		.layout = layout_symbol(mon->selws->layout),
		.workspace = mon->selws->name
	};
	char buf[1024];

	sysinfo_format(settings()->barfmt, buf, sizeof(buf), &fd);
	bar_draw(mon->bar, buf);

	free(fd.layout);
}

/** Set the floating state for the selected client */
void
monitor_float_selected(struct monitor *mon, bool floating)
{
	if (mon->sel) {
		client_set_floating(mon->sel, floating);
		monitor_arrange(mon);
	}
}

/** Focus on the given client. If no client given, the first available
 * client will be focused */
void
monitor_focus(struct monitor *mon, struct client *c)
{
	DBG("%s(%p, %p)\n", __func__, (void *)mon, (void *)c);

	if (!c || !ISFOCUSABLE(c))
		for (c = mon->cstack; c && !ISFOCUSABLE(c); c = c->snext);
	if (mon->sel && mon->sel != c)
		monitor_unfocus_selected(mon);
	if (c) {
		/* TODO: fix support for multiple monitors */

		/* move the window to the front of the client stack */
		remove_from_stack(mon, c);
		add_to_stack(mon, c);

		client_set_focus(c, true);
	} else {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
	}

	mon->sel = c;

	/* TODO draw bar */
	monitor_draw_bar(mon);
}

/** Frees the given monitor */
struct monitor *
monitor_free(struct monitor *mon)
{
	struct monitor *next = mon->next;

	bar_free(mon->bar);
	free(mon);

	return next;
}

/** Move the tiled clients to the places given by the layout */
void
monitor_move_clients(struct monitor *mon)
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

		client_move_resize(c, x, y, w, h, true, false);
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
monitor_restack(struct monitor *mon)
{
	struct client *c;
	XEvent ev;
	XWindowChanges wc;

	monitor_draw_bar(mon);

	if (!mon->sel)
		return;
	if (mon->sel->floating || !ISARRANGED(mon))
		client_raise(mon->sel);

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
monitor_select_client(struct monitor *mon, struct client *c, bool switch_to_ws)
{
	assert(c->mon == mon);

	if (!ISSELECTABLE(c))
		return;

	if (mon->selws_i != c->ws && c->ws != ALL_WS && !switch_to_ws)
		return;

	monitor_unfocus_selected(mon);
	mon->sel = c;

	if (mon->selws_i == c->ws || c->ws == ALL_WS) {
		monitor_focus(mon, c);
		monitor_restack(mon);
	} else {
		monitor_set_ws(mon, c->ws);
	}
}

/** Selects the next valid client in the client list */
void
monitor_select_next_client(struct monitor *mon)
{
	struct client *c = NULL;

	if (mon->sel && (c = next_selectable_client(mon->sel)))
		monitor_select_client(mon, c, false);
}

/** Selects the prevous valid client in the client list */
void
monitor_select_prev_client(struct monitor *mon)
{
	struct client *c = NULL;

	if (mon->sel && (c = prev_selectable_client(mon->sel)))
		monitor_select_client(mon, c, false);
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
monitor_set_layout(struct monitor *mon, int id)
{
	assert(id >= 0 && id < LASTLayout);

	layout_set(mon->selws->layout, id);
	monitor_arrange(mon);
}

/** Set the current workspace */
void
monitor_set_ws(struct monitor *mon, unsigned long ws)
{
	assert(VALID_WORKSPACE(ws));

	mon->prevws_i = mon->selws_i;
	mon->selws_i = ws;
	mon->selws = &mon->ws[ws];

	monitor_update_window_size(mon);
	monitor_focus(mon, NULL);
	monitor_arrange(mon);

	ewmh_root_set_current_desktop(mon->num * N_WORKSPACES + ws);
}

/** Show/hide the bar */
void
monitor_show_bar(struct monitor *mon, bool show)
{
	mon->bar->showbar = show ? 1 : 0;
	monitor_update_window_size(mon);
	XMoveResizeWindow(dpy, mon->bar->win, mon->wx, mon->bar->y,
			mon->ww, mon->bar->h);
	monitor_arrange(mon);
	monitor_draw_bar(mon);
}

/** Show/hide the clients */
void
monitor_show_hide(struct monitor *mon)
{
	show_hide(mon->cstack);
}

/** Swap the two clients */
void
monitor_swap(struct monitor *mon, struct client *c1, struct client *c2)
{
	struct client *tmp;

	assert(c1->mon == mon);
	assert(c2->mon == mon);

	if (c1 == c2)
		return;

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

	monitor_arrange(mon);
}

/** Swaps the currently selected client with the next tiled client */
void
monitor_swap_next_client(struct monitor *mon)
{
	struct client *sel = mon->sel;
	struct client *c = NULL;

	if (!sel || !ISTILED(sel))
		return;

	if (((c = next_tiled(sel->next)) || (c = next_tiled(mon->clients)))
			&& c != sel) {
		monitor_swap(mon, sel, c);
	}
}

/** Swaps the currently selected client with the previous tiled client */
void
monitor_swap_prev_client(struct monitor *mon)
{
	struct client *sel = mon->sel;
	struct client *c = NULL;

	if (!sel || !ISTILED(sel))
		return;

	if (((c = prev_tiled(sel->prev)) || (c = last_tiled(mon->clients)))
			&& c != sel) {
		monitor_swap(mon, sel, c);
	}
}

/** Removes focus from the currently selected client */
void
monitor_unfocus_selected(struct monitor *mon)
{
	if (mon->sel)
		client_set_focus(mon->sel, false);
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
find_client_by_trans(struct monitor *mon, Window win)
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

struct client *
find_nth_tiled_client(struct monitor *mon, int pos)
{
	struct client *c;
	for (c = next_tiled(mon->clients); c && pos > 0;
			c = next_tiled(c->next), pos--);
	return c;
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
