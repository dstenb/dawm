#include "cursors.h"

static Cursor cursors[LASTCursor];

Cursor
cursor(CursorID id)
{
	assert(id < LASTCursor);
	return cursors[id];
}

void
cursors_init(Display *dpy)
{
	cursors[NormalCursor] = XCreateFontCursor(dpy, XC_left_ptr);
	cursors[MovementCursor] = XCreateFontCursor(dpy, XC_fleur);
	cursors[ResizeCursor] = XCreateFontCursor(dpy, XC_sizing);
}

void
cursors_free(Display *dpy)
{
	int i;

	for (i = 0; i < LASTCursor; i++)
		XFreeCursor(dpy, cursors[i]);
}
