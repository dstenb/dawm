#include "bar.h"

#define EVENT_MASK (CWOverrideRedirect | CWBackPixmap | CWEventMask)

struct bar *bar_create(int topbar, int showbar, int x, int y, int w, int h,
		Display *dpy, Window root, int screen)
{
	struct bar *bar;
	XSetWindowAttributes attr = {
		.override_redirect = True,
		.background_pixmap = ParentRelative,
		.event_mask = ExposureMask
	};

	bar = xcalloc(1, sizeof(struct bar));

	/* TODO */

	bar->topbar = topbar;
	bar->showbar = showbar;
	bar->x = x;
	bar->y = y;
	bar->w = w;
	bar->h = h;

	return bar;

	bar->drawable = XCreatePixmap(dpy, root, bar->w, bar->h,
			DefaultDepth(dpy, screen));

	bar->gc =XCreateGC(dpy, root, 0, NULL);

	bar->win = XCreateWindow(dpy, root, bar->x, bar->y, bar->w, bar->h, 0,
			DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen),
			EVENT_MASK, &attr);

	XDefineCursor(dpy, bar->win, cursor(NormalCursor));
	XMapRaised(dpy, bar->win);

	return bar;
}

void bar_draw(struct bar *bar, Display *dpy)
{

}
