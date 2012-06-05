#include "wm.h"

static unsigned long wm_getcolor(struct wm *, const char *);

unsigned long wm_getcolor(struct wm *wm, const char *str) {
	Colormap cmap = DefaultColormap(wm->dpy, wm->screen);
	XColor color;

	if(!XAllocNamedColor(wm->dpy, cmap, str, &color, &color))
		die("error, cannot allocate color '%s'\n", str);
	return color.pixel;
}

struct wm *wm_init(void)
{
	struct wm *wm;

	if (!(wm = calloc(1, sizeof(struct wm))))
		die("couldn't malloc\n");

	if (!(wm->dpy = XOpenDisplay(NULL)))
		die("couldn't open display '%s'\n", getenv("DISPLAY"));

	/* TODO check for other window managers */

	wm->screen = DefaultScreen(wm->dpy);
	wm->root = RootWindow(wm->dpy, wm->screen);
	wm->width = DisplayWidth(wm->dpy, wm->screen);
	wm->height = DisplayHeight(wm->dpy, wm->screen);

	printf("wm->width: %i\n", wm->width);
	printf("wm->height: %i\n", wm->height);

	wm->restart = 0;

	return wm;
}

int wm_eventloop(struct wm *wm)
{
	(void)wm;
	return 0;
}

int wm_destroy(struct wm *wm)
{
	free(wm);
	return 0;
}
