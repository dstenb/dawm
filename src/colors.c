#include "dawm.h"

static const char *colorstr[] = {
	"bar_border",
	"bar_norm_fg",
	"bar_norm_bg",
	"bar_sel_fg",
	"bar_sel_bg",
	"win_norm_border",
	"win_sel_border"
};

const char *
color_id2str(ColorID id)
{
	return (id >= 0 && id < LASTColor) ? colorstr[id] : NULL;
}

int
color_str2id(const char *str)
{
	int i;

	for (i = 0; i < ARRSIZE(colorstr); i++) {
		if (colorstr[i] && STREQ(colorstr[i], str))
			return i;
	}

	return InvalidColor;
}

#ifdef XFT
void
color_alloc_xft(const struct color *c, XftColor *xftc)
{
	XRenderColor xrc = { .red = c->r, .green = c->g,
		.blue = c->b,.alpha = 0xFFFF
	};

	XftColorAllocValue(dpy, DefaultVisual(dpy, screen),
			DefaultColormap(dpy, screen), &xrc, xftc);
}
#endif

void
color_alloc_xlib(const struct color *c, unsigned long *value)
{
	Colormap cmap = DefaultColormap(dpy, screen);
	XColor cell = { .flags = DoRed | DoGreen | DoBlue,
		.red = c->r,
		.green = c->g,
		.blue = c->b
	};

	if(!XAllocColor(dpy, cmap, &cell))
		die("error, cannot allocate color '%s'\n", color_string(c));

	*value = cell.pixel;
}

static unsigned short
hex(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'z')
		return 0xA + (c - 'a');
	else
		return 0xA + (c - 'A');
}

bool
color_parse(const char *str, struct color *c)
{
	if (strmatch("^#[0-9A-Fa-f]{6}", str)) {
		c->r = hex(str[1]) << 4 | hex(str[2]);
		c->g = hex(str[3]) << 4 | hex(str[4]);
		c->b = hex(str[5]) << 4 | hex(str[6]);
		return true;
	}
	return false;
}

char *
color_string(const struct color *c)
{
	char *str = xcalloc(7, sizeof(char));
	sprintf(str, "#%02X%02X%02X", c->r, c->g, c->b);
	return str;
}
