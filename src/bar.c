#include "dawm.h"

#define EVENT_MASK (CWOverrideRedirect | CWBackPixmap | CWEventMask)

static void bar_copy_area(struct bar *);
static void bar_draw_bg(struct bar *);
static void bar_draw_text(struct bar *, char *, int);
static void bars_init_dc(void);
static void bars_init_font(const char *);

static struct {
#ifdef XFT
	XftColor fg_color;
	XftColor bg_color;
#else
	Drawable drawable;
	GC gc;
#endif
} dc;

static struct {
	int height;
#ifdef XFT
	XftFont *xfont;
#else
	int ascent;
	int descent;
	XFontSet set;
	XFontStruct *xfont;
#endif
} font;

static bool initialized = false;

void
bar_copy_area(struct bar *bar)
{
#ifdef XFT
#else
	XCopyArea(dpy, dc.drawable, bar->win, dc.gc,
			0, 0, bar->w, bar->h, 0, 0);
	XSync(dpy, False);

#endif
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
	bar->h = font.height + 2 * BAR_PADDING_Y;

	bar->win = XCreateWindow(dpy, root, bar->x, bar->y, bar->w, bar->h, 0,
			DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen),
			EVENT_MASK, &attr);

#ifdef XFT
	bar->xftdraw = XftDrawCreate(dpy, bar->win, DefaultVisual(dpy, screen),
			DefaultColormap(dpy, screen));
#endif

	cursor_set(bar->win, NormalCursor);
	XMapRaised(dpy, bar->win);

	return bar;
}

void
bar_draw(struct bar *bar, char *str)
{
	bar_draw_bg(bar);
	bar_draw_text(bar, str, strlen(str));
	bar_copy_area(bar);
}

void
bar_draw_bg(struct bar *bar)
{
#ifdef XFT
	XftDrawRect(bar->xftdraw, &dc.bg_color, 0, 0, bar->w, bar->h);
#else
	XSetForeground(dpy, dc.gc, color(BarNormBG));
	XFillRectangle(dpy, dc.drawable, dc.gc, 0, 0, bar->w, bar->h);

#endif
}

void
bar_draw_text(struct bar *bar, char *str, int len)
{
#ifdef XFT
	XGlyphInfo info;

	XftTextExtents8(dpy, font.xfont, (XftChar8 *)str, len, &info);

	/* TODO: fix y */
	int y = (bar->h - (info.height - info.y) + info.height) / 2;
	XftDrawString8(bar->xftdraw, &dc.fg_color, font.xfont, BAR_PADDING_X,
			y, (XftChar8 *)str, len);
#else
	int x = BAR_PADDING_X;
	int y = (bar->h / 2) - (font.height / 2) + font.ascent;

	XSetForeground(dpy, dc.gc, color(BarNormFG));

	if(font.set)
		XmbDrawString(dpy, dc.drawable, font.set, dc.gc,
				x, y, str, len);
	else
		XDrawString(dpy, dc.drawable, dc.gc, x, y, str, len);
#endif
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

#ifdef XFT
static void
init_xft_color(unsigned short r, unsigned g, unsigned b,
		unsigned short a, XftColor *xftc)
{
	XRenderColor xrc = { .red = r, .green = g, .blue = b, .alpha = a };

	XftColorAllocValue(dpy, DefaultVisual(dpy, screen),
			DefaultColormap(dpy, screen), &xrc, xftc);
}
#endif

void
bars_init_dc(void)
{
#ifdef XFT
	init_xft_color(0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, &dc.fg_color);
	init_xft_color(0x0000, 0x0000, 0x0000, 0xFFFF, &dc.bg_color);
#else
	dc.drawable = XCreatePixmap(dpy, root, screen_w, font.height,
			DefaultDepth(dpy, screen));
	dc.gc = XCreateGC(dpy, root, 0, NULL);
#endif
}

void
bars_init_font(const char *fontstr)
{
#ifdef XFT
	XGlyphInfo info;

	font.xfont = XftFontOpenName(dpy, screen, fontstr);
	/* TODO: fix this ;) */
	XftTextExtents8(dpy, font.xfont, (XftChar8 *)"A", 1, &info);
	font.height = info.height;
#else
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
#endif
}

void
bars_free(void)
{
	if (initialized) {
#ifdef XFT
#else
		if (font.set)
			XFreeFontSet(dpy, font.set);
		else
			XFreeFont(dpy, font.xfont);

		XFreePixmap(dpy, dc.drawable);
		XFreeGC(dpy, dc.gc);
#endif
	}
}
