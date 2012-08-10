#ifndef _CURSORS_H_
#define _CURSORS_H_

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/cursorfont.h>
#include <X11/Xlib.h>

#include "x11.h"

typedef enum {
	NormalCursor,
	MovementCursor,
	ResizeCursor,
	LASTCursor
} CursorID;

Cursor cursor(CursorID);
void cursor_set(Window, CursorID);
void cursors_init(void);
void cursors_free(void);

#endif
