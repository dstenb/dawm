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
cursor_set(Window win, CursorID id)
{
	XDefineCursor(dpy, win, cursor(id));
}

/** Initializes the cursors */
void
cursors_init(void)
{
	cursors[NormalCursor] = XCreateFontCursor(dpy, XC_left_ptr);
	cursors[MovementCursor] = XCreateFontCursor(dpy, XC_fleur);
	cursors[ResizeCursor] = XCreateFontCursor(dpy, XC_sizing);
}

/** Free all the allocated cursors */
void
cursors_free(void)
{
	int i;

	for (i = 0; i < LASTCursor; i++)
		XFreeCursor(dpy, cursors[i]);
}
