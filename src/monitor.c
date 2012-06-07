#include "monitor.h"

void monitor_add_client(struct monitor *mon, struct client *c)
{
	c->mon = mon;
	c->next = mon->clients;
	mon->clients = c;

	c->snext = mon->cstack;
	mon->cstack = c;
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

void monitor_arrange(struct monitor *mon)
{
	/* TODO */
}

struct monitor *monitor_create(struct config *cfg, int width, int height)
{
	struct monitor *mon = xcalloc(1, sizeof(struct monitor));

	/*m->bar = bar_create(cfg);*/
	mon->clients = NULL;
	mon->cstack = NULL;
	mon->sel = NULL;

	mon->next = NULL;

	mon->width = width;
	mon->height = height;

	return mon;
}

void monitor_focus(struct monitor *mon, struct client *c)
{
	/* TODO */
}

void monitor_select_client(struct monitor *mon, struct client *c)
{
	c->mon->sel = c;
}

struct client *find_client_by_window(struct monitor *mon, Window win)
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
