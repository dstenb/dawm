#include "dawm.h"

static unsigned long get_color(const char *);

static unsigned long colors[LASTColor];

static const char *colorstr[] = {
	"bar_border",
	"bar_norm_fg",
	"bar_norm_bg",
	"bar_sel_fg",
	"bar_sel_bg",
	"win_norm_border",
	"win_sel_border"
};

unsigned long
get_color(const char *str)
{
	Colormap cmap = DefaultColormap(dpy, screen);
	XColor c;

	if(!XAllocNamedColor(dpy, cmap, str, &c, &c))
		die("error, cannot allocate color '%s'\n", str);
	return c.pixel;
}

/* get color with the given id */
unsigned long
color(ColorID id)
{
	assert(id < LASTColor);
	return colors[id];
}

/* initializes the colors */
void
colors_init(char *const str[LASTColor])
{
	int i;

	for (i = 0; i < LASTColor; i++)
		colors[i] = get_color(str[i]);
}

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
