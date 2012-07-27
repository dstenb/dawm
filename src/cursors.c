#include "cursors.h"

static Cursor cursors[LASTCursor];

/** Get cursor with the given id */
Cursor
cursor(CursorID id)
{
	assert(id < LASTCursor);
	return cursors[id];
}

void
cursor_set(Window win, CursorID id, Display *dpy)
{
	XDefineCursor(dpy, win, cursor(id));
}

/** Initializes the cursors */
void
cursors_init(Display *dpy)
{
	cursors[NormalCursor] = XCreateFontCursor(dpy, XC_left_ptr);
	cursors[MovementCursor] = XCreateFontCursor(dpy, XC_fleur);
	cursors[ResizeCursor] = XCreateFontCursor(dpy, XC_sizing);
}

/** Free all the allocated cursors */
void
cursors_free(Display *dpy)
{
	int i;

	for (i = 0; i < LASTCursor; i++)
		XFreeCursor(dpy, cursors[i]);
}
