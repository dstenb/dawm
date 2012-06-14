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

/* get cursor with the given id */
Cursor cursor(CursorID);

/* initializes the cursors */
void cursors_init(Display *);

/* free al the allocated cursors */
void cursors_free(Display *);

#endif
