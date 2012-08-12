#include "dawm.h"

#define MW(N, NM, W, MF) ((N > NM) ? (NM ? (W * MF) : 0) : W)
#define MH(N, NM, H, MF) ((N > NM) ? (NM ? (H * MF) : 0) : H)

/** Callback struct */
struct layout_cb {
	void (*arrange) (struct layout *);
	void (*client_added) (struct layout *);
	void (*client_removed) (struct layout *);
	void (*clients_changed) (struct layout *, unsigned);
	void (*geom_changed) (struct layout *, int, int);
	void (*mfact_changed) (struct layout *, float);
	void (*nmaster_changed) (struct layout *, int);
};

void layout_allocate_pos(struct layout *, unsigned);

/* Horizontal layout functions */
void horz_arrange(struct layout *);
void horz_client_added(struct layout *);
void horz_client_removed(struct layout *);
void horz_clients_changed(struct layout *, unsigned);
void horz_geom_changed(struct layout *, int, int);
void horz_mfact_changed(struct layout *, float);
void horz_nmaster_changed(struct layout *, int);

/* Vertical layout functions */
void vert_arrange(struct layout *);
void vert_client_added(struct layout *);
void vert_client_removed(struct layout *);
void vert_clients_changed(struct layout *, unsigned);
void vert_geom_changed(struct layout *, int, int);
void vert_mfact_changed(struct layout *, float);
void vert_nmaster_changed(struct layout *, int);

/* Max layout functions */
void max_arrange(struct layout *);
void max_client_added(struct layout *);
void max_client_removed(struct layout *);
void max_clients_changed(struct layout *, unsigned);
void max_geom_changed(struct layout *, int, int);

/** Callbacks */
static struct layout_cb callbacks[LASTLayout] = {
	{
		.arrange = horz_arrange,
		.client_added = horz_client_added,
		.client_removed = horz_client_removed,
		.clients_changed = horz_clients_changed,
		.geom_changed = horz_geom_changed,
		.mfact_changed = horz_mfact_changed,
		.nmaster_changed = horz_nmaster_changed
	},
	{
		.arrange = vert_arrange,
		.client_added = vert_client_added,
		.client_removed = vert_client_removed,
		.clients_changed = vert_clients_changed,
		.geom_changed = vert_geom_changed,
		.mfact_changed = vert_mfact_changed,
		.nmaster_changed = vert_nmaster_changed
	},
	{
		.arrange = max_arrange,
		.client_added = max_client_added,
		.client_removed = max_client_removed,
		.clients_changed = max_clients_changed,
		.geom_changed = max_geom_changed,
		.mfact_changed = NULL,
		.nmaster_changed = NULL
	}
};

void
horz_arrange(struct layout *layout)
{
	struct layout_pos *pos;
	unsigned int i, mw, my, ty;

	mw = MW(layout->n, layout->nmaster, layout->ww, layout->mfact);

	for (i = my = ty = 0, pos = layout->pos; i < layout->n; i++, pos++) {
		if (i < layout->nmaster) {
			pos->x = 0;
			pos->y = my;
			pos->w = mw;
			pos->h = (layout->wh - my) /
				(MIN(layout->n, layout->nmaster) - i);
			my += pos->h;
		} else {
			pos->x = mw;
			pos->y = ty;
			pos->w = layout->ww - mw;
			pos->h = (layout->wh - ty) / (layout->n - i);
			ty += pos->h;
		}
	}
}

void
horz_client_added(struct layout *layout)
{
	/* TODO */
	layout_allocate_pos(layout, layout->n + 1);
	horz_arrange(layout);
}

void
horz_client_removed(struct layout *layout)
{
	/* TODO */
	if (layout->n > 0) {
		layout_allocate_pos(layout, layout->n - 1);
		horz_arrange(layout);
	}
}

void
horz_clients_changed(struct layout *layout, unsigned n)
{
	/* TODO */
	layout_allocate_pos(layout, n);
	horz_arrange(layout);
}

void
horz_geom_changed(struct layout *layout, int ww, int wh)
{
	/* TODO: handle width and height diffs instead of re-arranging */
	layout->ww = ww;
	layout->wh = wh;

	horz_arrange(layout);
}

void
horz_mfact_changed(struct layout *layout, float mfact)
{
	struct layout_pos *pos;
	unsigned int i, mw;

	mw = MW(layout->n, layout->nmaster, layout->ww, mfact);

	for (i = 0, pos = layout->pos; i < layout->n; i++, pos++) {
		if (i < layout->nmaster) {
			pos->w = mw;
		} else {
			pos->x = mw;
			pos->w = layout->ww - mw;
		}
	}
}

void
horz_nmaster_changed(struct layout *layout, int nmaster)
{
	/* TODO */
}

void
vert_arrange(struct layout *layout)
{
	struct layout_pos *pos;
	unsigned int i, mh, mx, tx;

	mh = MH(layout->n, layout->nmaster, layout->wh, layout->mfact);

	for (i = mx = tx = 0, pos = layout->pos; i < layout->n; i++, pos++) {
		if (i < layout->nmaster) {
			pos->x = mx;
			pos->y = 0;
			pos->w = (layout->ww - mx) /
				(MIN(layout->n, layout->nmaster) - i);
			pos->h = mh;
			mx += pos->w;
		} else {
			pos->x = tx;
			pos->y = mh;
			pos->w = (layout->ww - tx) / (layout->n - i);
			pos->h = layout->wh - mh;
			tx += pos->w;
		}
	}
}

void
vert_client_added(struct layout *layout)
{
	/* TODO */
	layout_allocate_pos(layout, layout->n + 1);
	vert_arrange(layout);
}

void
vert_client_removed(struct layout *layout)
{
	/* TODO */
	if (layout->n > 0) {
		layout_allocate_pos(layout, layout->n - 1);
		vert_arrange(layout);
	}
}

void
vert_clients_changed(struct layout *layout, unsigned n)
{
	/* TODO */
	layout_allocate_pos(layout, n);
	vert_arrange(layout);
}

void
vert_geom_changed(struct layout *layout, int ww, int wh)
{
	/* TODO: handle width and height diffs instead of re-arranging */
	layout->ww = ww;
	layout->wh = wh;

	vert_arrange(layout);
}

void
vert_mfact_changed(struct layout *layout, float mfact)
{
	struct layout_pos *pos;
	unsigned int i, mh;

	mh = MH(layout->n, layout->nmaster, layout->wh, mfact);

	for (i = 0, pos = layout->pos; i < layout->n; i++, pos++) {
		if (i < layout->nmaster) {
			pos->h = mh;
		} else {
			pos->y = mh;
			pos->h = layout->wh - mh;
		}
	}
}

void
vert_nmaster_changed(struct layout *layout, int nmaster)
{
	/* TODO */
}

void
max_arrange(struct layout *layout)
{
	struct layout_pos *pos;
	unsigned int i = 0;

	for (i = 0, pos = layout->pos; i < layout->n; i++, pos++) {
		pos->x = pos->y = 0;
		pos->w = layout->ww;
		pos->h = layout->wh;
	}
}

void
max_client_added(struct layout *layout)
{
	layout_allocate_pos(layout, layout->n + 1);
	max_arrange(layout);
}

void
max_client_removed(struct layout *layout)
{
	if (layout->n > 0) {
		layout_allocate_pos(layout, layout->n - 1);
		max_arrange(layout);
	}
}

void
max_clients_changed(struct layout *layout, unsigned n)
{
	layout_allocate_pos(layout, n);
	max_arrange(layout);
}

void
max_geom_changed(struct layout *layout, int ww, int wh)
{
	/* TODO: handle width and height diffs instead of re-arranging */
	layout->ww = ww;
	layout->wh = wh;

	max_arrange(layout);
}

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
layout_add_client(struct layout *layout)
{
	if (callbacks[layout->id].client_added)
		callbacks[layout->id].client_added(layout);
}

void
layout_allocate_pos(struct layout *layout, unsigned n)
{
	if (layout->pos)
		free(layout->pos);
	layout->pos = xcalloc(n, sizeof(struct layout_pos));
	layout->n = n;
}

int
layout_pos_index(struct layout *layout, int x, int y)
{
	struct layout_pos *pos;
	unsigned int i;

	for (i = 0, pos = layout->pos; i < layout->n; i++, pos++) {
		if (INSIDE(x, y, pos->x, pos->y, pos->w, pos->h))
			return i;
	}

	return -1;
}

void
layout_remove_client(struct layout *layout)
{
	if (callbacks[layout->id].client_removed)
		callbacks[layout->id].client_removed(layout);
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
	mfact = MAX(0.01, MIN(0.99, mfact));

	if (callbacks[layout->id].mfact_changed)
		callbacks[layout->id].mfact_changed(layout, mfact);

	layout->mfact = mfact;
}

void
layout_set_nmaster(struct layout *layout, unsigned nmaster)
{
	nmaster = MIN(4, nmaster);

	if (callbacks[layout->id].nmaster_changed)
		callbacks[layout->id].nmaster_changed(layout, nmaster);

	layout->nmaster = nmaster;
}

int
layout_str2id(const char *str)
{
	if (STREQ(str, "horz"))
		return TileHorzLayout;
	else if (STREQ(str, "vert"))
		return TileVertLayout;
	else if (STREQ(str, "max"))
		return MaxLayout;
	return -1;
}

const char *
layout_symbol(const struct layout *layout)
{
	static const char *symbols[] = { "|", "-", "M" };

	return (layout->id < LASTLayout) ? symbols[layout->id] : "";
}
