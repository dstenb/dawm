#include "bar.h"

#define EVENT_MASK (CWOverrideRedirect | CWBackPixmap | CWEventMask)

static struct {
	Drawable drawable;
	GC gc;
} dc;

static int initialized = 0;

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

	/* TODO: move this to a better place */
	if (!initialized)
		bars_init_dc(dpy, root, screen, h);

	bar->topbar = topbar;
	bar->showbar = showbar;
	bar->x = x;
	bar->y = y;
	bar->w = w;
	bar->h = h;

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
	XSetForeground(dpy, dc.gc, color(BarNormBG));
	XFillRectangle(dpy, dc.drawable, dc.gc, 0, 0, bar->w, bar->h);
	XCopyArea(dpy, dc.drawable, bar->win, dc.gc, 0, 0, bar->w, bar->h, 0, 0);
	XSync(dpy, False);
}

void bars_init_dc(Display *dpy, Window root, int screen, int h)
{
	if (!initialized) {
		dc.drawable = XCreatePixmap(dpy, root,
				DisplayWidth(dpy, screen), h,
				DefaultDepth(dpy, screen));
		dc.gc = XCreateGC(dpy, root, 0, NULL);
		initialized = 1;
	}
}
