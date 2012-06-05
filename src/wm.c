#include "wm.h"

static unsigned long wm_getcolor(struct wm *, const char *);
static void wm_checkotherwm(struct wm *);
static int wm_xerror_checkotherwm(Display *, XErrorEvent *);
static int wm_xerror(Display *, XErrorEvent *);

void wm_checkotherwm(struct wm *wm)
{
	XSetErrorHandler(wm_xerror_checkotherwm);
	/* this causes an error if some other window manager is running */
	XSelectInput(wm->dpy, DefaultRootWindow(wm->dpy),
			SubstructureRedirectMask);
	XSync(wm->dpy, False);
	XSetErrorHandler(wm_xerror);
	XSync(wm->dpy, False);
}

int wm_xerror(Display *dpy, XErrorEvent *ee)
{

}

int wm_xerror_checkotherwm(Display *dpy, XErrorEvent *ee)
{
	die("another window manager is running\n");
	return 0;
}

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

	/* check if another wm is running */
	wm_checkotherwm(wm);

	wm->screen = DefaultScreen(wm->dpy);
	wm->root = RootWindow(wm->dpy, wm->screen);
	wm->width = DisplayWidth(wm->dpy, wm->screen);
	wm->height = DisplayHeight(wm->dpy, wm->screen);
	wm->restart = 0;

	printf("wm->width: %i\n", wm->width);
	printf("wm->height: %i\n", wm->height);


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
