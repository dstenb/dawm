#include "layouts.h"

static void
layout_allocate_pos(struct layout *layout, unsigned n)
{
	if (layout->pos)
		free(layout->pos);
	layout->pos = xcalloc(n, sizeof(struct layout_pos));
	layout->n = n;
}

struct layout_cb {
	void (*arrange) (struct layout *);
	void (*clients_changed) (struct layout *, unsigned);
	void (*geom_changed) (struct layout *, int, int);
	void (*mfact_changed) (struct layout *);
	void (*nmaster_changed) (struct layout *);
};

/*** Horizontal layout functions ***/
static void
horz_arrange(struct layout *layout)
{
	struct layout_pos *pos;
	unsigned int mw, my, ty, h;
	unsigned int i = 0;

	if (layout->n > layout->nmaster)
		mw = layout->nmaster ? layout->ww * layout->mfact : 0;
	else
		mw = layout->ww;

	for (i = my = ty = 0; i < layout->n; i++) {
		pos = &layout->pos[i];

		if (i < layout->nmaster) {
			pos->x = 0;
			pos->y = my;
			pos->w = mw;
			pos->h = (layout->wh - my) /
				(MIN(layout->n, layout->nmaster) - i);
			my += pos->h;
		} else {
			h = (layout->wh - ty) / (layout->n - i);
			pos->x = mw;
			pos->y = ty;
			pos->w = mw;
			pos->h = h;
			ty += pos->h;
		}
	}
}

static void
horz_clients_changed(struct layout *layout, unsigned n)
{
	/* TODO */
	layout_allocate_pos(layout, n);
	horz_arrange(layout);
}

static void
horz_geom_changed(struct layout *layout, int ww, int wh)
{
	/* TODO: handle width and height diffs instead of re-arranging */
	layout->ww = ww;
	layout->wh = wh;

	horz_arrange(layout);
}

static void
horz_mfact_changed(struct layout *layout)
{
	/* TODO */
}

static void
horz_nmaster_changed(struct layout *layout)
{
	/* TODO */
}

static struct layout_cb callbacks[LASTLayout] = {
	{
		.arrange = horz_arrange,
		.clients_changed = horz_clients_changed,
		.geom_changed = horz_geom_changed,
		.mfact_changed = horz_mfact_changed,
		.nmaster_changed = horz_nmaster_changed
	}
};



struct layout *
layout_init(LayoutID id, int mw, int mh, int ww, int wh,
		int nmaster, float mfact)
{
	struct layout *layout = xcalloc(1, sizeof(struct layout));

	layout->pos = NULL;
	layout->n = 0;
	layout->mw = mw;
	layout->mh = mh;
	layout->ww = ww;
	layout->wh = wh;
	layout->mfact = mfact;
	layout->nmaster = nmaster;

	layout->id = id;

	return layout;
}

void
layout_set(struct layout *layout, LayoutID id)
{
	assert(id < LASTLayout);
	layout->id = id;

	if (callbacks[layout->id].arrange)
		callbacks[layout->id].arrange(layout);
}

void
layout_set_clients(struct layout *layout, unsigned n)
{
	if (callbacks[layout->id].clients_changed)
		callbacks[layout->id].clients_changed(layout, n);
}

void
layout_set_geom(struct layout *layout, int ww, int wh)
{
	if (callbacks[layout->id].geom_changed)
		callbacks[layout->id].geom_changed(layout, ww, wh);

	layout->ww = ww;
	layout->wh = wh;
}

void
layout_set_mfact(struct layout *layout, float mfact)
{
#if 0
	layout->mfact = MAX(0.01, MIN(0.99, mfact));
	if (callbacks[layout->id].mfact_changed)
		callbacks[layout->id].mfact_changed(layout);
#endif
}

void
layout_set_nmaster(struct layout *layout, unsigned nmaster)
{
#if 0
	layout->nmaster = MAX(1, MIN(4, nmaster));
	if (callbacks[layout->id].nmaster_changed)
		callbacks[layout->id].nmaster_changed(layout);
#endif
}

const char *
layout_symbol(const struct layout *layout)
{
	switch(layout->id) {
		case TileHorzLayout:
			return "|";
		case TileVertLayout:
			return "-";
		case MaxLayout:
			return "M";
		default:
			return "";
	}
}
