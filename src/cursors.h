#ifndef _CURSORS_H_
#define _CURSORS_H_

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/cursorfont.h>
#include <X11/Xlib.h>

typedef enum {
	NormalCursor,
	MovementCursor,
	ResizeCursor,
	LASTCursor
} CursorID;

Cursor cursor(CursorID);
void cursors_init(Display *);
void cursors_free(Display *);

#endif
