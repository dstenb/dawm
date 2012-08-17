#include "dawm.h"

#define EVENT_MASK (CWOverrideRedirect | CWBackPixmap | CWEventMask)

static void bar_copy_area(struct bar *);
static void bar_draw_bg(struct bar *);
static void bar_draw_text(struct bar *, const char *, int);
static void bars_init_dc(void);
static void bars_init_font(const char *);

static struct {
	Drawable drawable;
	GC gc;
} dc;

static struct {
	int ascent;
	int descent;
	int height;
	XFontSet set;
	XFontStruct *xfont;
} font;

static bool initialized = false;

void
bar_copy_area(struct bar *bar)
{
	XCopyArea(dpy, dc.drawable, bar->win, dc.gc,
			0, 0, bar->w, bar->h, 0, 0);
	XSync(dpy, False);
}

struct bar *
bar_create(bool topbar, bool showbar, int x, int y, int w)
{
	struct bar *bar;
	XSetWindowAttributes attr = {
		.override_redirect = True,
		.background_pixmap = ParentRelative,
		.event_mask = ExposureMask
	};

	bar = xcalloc(1, sizeof(struct bar));

	bar->topbar = topbar;
	bar->showbar = showbar;
	bar->x = x;
	bar->y = y;
	bar->w = w;
	bar->h = font.height + 2;

	bar->win = XCreateWindow(dpy, root, bar->x, bar->y, bar->w, bar->h, 0,
			DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen),
			EVENT_MASK, &attr);

	cursor_set(bar->win, NormalCursor);
	XMapRaised(dpy, bar->win);

	return bar;
}

void
bar_draw(struct bar *bar, const char *str)
{
	bar_draw_bg(bar);
	bar_draw_text(bar, str, strlen(str));
	bar_copy_area(bar);
}

void
bar_draw_bg(struct bar *bar)
{
	XSetForeground(dpy, dc.gc, color(BarNormBG));
	XFillRectangle(dpy, dc.drawable, dc.gc, 0, 0, bar->w, bar->h);
}

void
bar_draw_text(struct bar *bar, const char *str, int len)
{
	int x = (font.height) / 2;
	int y = (bar->h / 2) - (font.height / 2) + font.ascent;

	XSetForeground(dpy, dc.gc, color(BarNormFG));

	if(font.set)
		XmbDrawString(dpy, dc.drawable, font.set, dc.gc,
				x, y, str, len);
	else
		XDrawString(dpy, dc.drawable, dc.gc, x, y, str, len);
}

void
bar_free(struct bar *bar)
{
	XUnmapWindow(dpy, bar->win);
	XDestroyWindow(dpy, bar->win);
	free(bar);
}

void
bars_init(const char *fontstr)
{
	if (!initialized) {
		bars_init_font(fontstr);
		bars_init_dc();
		initialized = true;
	}
}

void
bars_init_dc(void)
{
	dc.drawable = XCreatePixmap(dpy, root, screen_w, font.height,
			DefaultDepth(dpy, screen));
	dc.gc = XCreateGC(dpy, root, 0, NULL);
}

void
bars_init_font(const char *fontstr)
{
	char *def, **missing;
	int n;

	font.set = XCreateFontSet(dpy, fontstr, &missing, &n, &def);

	if(missing) {
		while(n--)
			error("missing fontset: %s\n", missing[n]);
		XFreeStringList(missing);
	}

	if(font.set) {
		XFontStruct **xfonts;
		char **font_names;

		font.ascent = font.descent = 0;
		XExtentsOfFontSet(font.set);
		n = XFontsOfFontSet(font.set, &xfonts, &font_names);

		while(n--) {
			font.ascent = MAX(font.ascent, (*xfonts)->ascent);
			font.descent = MAX(font.descent,(*xfonts)->descent);
			xfonts++;
		}
	} else {
		if(!(font.xfont = XLoadQueryFont(dpy, fontstr)) &&
				!(font.xfont = XLoadQueryFont(dpy, "fixed")))
			die("error, cannot load font: '%s'\n", fontstr);
		font.ascent = font.xfont->ascent;
		font.descent = font.xfont->descent;
	}

	font.height = font.ascent + font.descent;
}

void
bars_free(void)
{
	if (initialized) {
		if (font.set)
			XFreeFontSet(dpy, font.set);
		else
			XFreeFont(dpy, font.xfont);

		XFreePixmap(dpy, dc.drawable);
		XFreeGC(dpy, dc.gc);
	}
}
