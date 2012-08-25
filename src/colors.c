#include "dawm.h"

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

#ifdef XFT
void
color_alloc_xft(const struct color *c, XftColor *xftc)
{
	XRenderColor xrc = { .red = c->r, .green = c->g,
		.blue = c->b, .alpha = 0xFFFF
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
		.red = c->r, .green = c->g, .blue = c->b
	};

	if(!XAllocColor(dpy, cmap, &cell))
		die("couldn't allocate color '%s'\n", color_string_long(c));

	*value = cell.pixel;
}

bool
color_parse(const char *str, struct color *c)
{
#define HEX "[0-9A-Fa-f]"

	if (strmatch("^#"HEX"{6}$", str)) {
		c->r = (hex(str[1]) << 4 | hex(str[2])) << 8;
		c->g = (hex(str[3]) << 4 | hex(str[4])) << 8;
		c->b = (hex(str[5]) << 4 | hex(str[6])) << 8;
		return true;
	} else if (strmatch("^"HEX"{4}\\/"HEX"{4}\\/"HEX"{4}$", str)) {
		c->r = hex(str[0]) << 12 | hex(str[1]) << 8 |
			hex(str[2]) << 4 | hex(str[3]);
		c->g = hex(str[5]) << 12 | hex(str[6]) << 8 |
			hex(str[7]) << 4 | hex(str[8]);
		c->b = hex(str[10]) << 12 | hex(str[11]) << 8 |
			hex(str[12]) << 4 | hex(str[13]);
		return true;
	} else {
		return false;
	}
}

char *
color_string_long(const struct color *c)
{
	char *str = xcalloc(15, sizeof(char));
	sprintf(str, "%04X/%04X/%04X", c->r, c->g, c->b);
	return str;
}

char *
color_string_short(const struct color *c)
{
	char *str = xcalloc(8, sizeof(char));
	sprintf(str, "#%02X%02X%02X", c->r >> 8, c->g >> 8, c->b >> 8);
	return str;
}
