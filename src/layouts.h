#ifndef _LAYOUTS_H_
#define _LAYOUTS_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

/* layouts.h
 *
 * The layout struct handles all arrangement functionality. This
 * functionality is monitor and client independent, i.e.
 * adjustments for monitor offset and border width isn't handled here.
 */

#define layout_dec_mfact(L) layout_set_mfact((L), (L)->mfact - M_FACTSTEP);
#define layout_inc_mfact(L) layout_set_mfact((L), (L)->mfact + M_FACTSTEP);

#define layout_dec_nmaster(L) layout_set_nmaster(L, (L)->nmaster - 1);
#define layout_inc_nmaster(L) layout_set_nmaster(L, (L)->nmaster + 1);

typedef enum {
	TileHorzLayout,
	TileVertLayout,
	MaxLayout,
	LASTLayout
} LayoutID;

struct layout_pos {
	int x;
	int y;
	int w;
	int h;
};

struct layout {
	struct layout_pos *pos;
	unsigned n;
	int mw, mh;
	int ww, wh;
	float mfact;
	unsigned nmaster;
	LayoutID id;
};

struct layout *layout_init(LayoutID, int, int, int, int, int, float);

void layout_add_client(struct layout *);
void layout_remove_client(struct layout *);
void layout_set(struct layout *, LayoutID);
void layout_set_clients(struct layout *, unsigned);
void layout_set_geom(struct layout *, int, int);
void layout_set_mfact(struct layout *, float);
void layout_set_nmaster(struct layout *, unsigned);

int layout_str2id(const char *);

const char *layout_symbol(const struct layout *);

#endif
