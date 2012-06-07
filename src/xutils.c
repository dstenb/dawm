#include "xutils.h"

unsigned long get_color(Display *dpy, int screen, const char *str)
{
	Colormap cmap = DefaultColormap(dpy, screen);
	XColor color;

	if(!XAllocNamedColor(dpy, cmap, str, &color, &color))
		die("error, cannot allocate color '%s'\n", str);
	return color.pixel;
}

int get_text_prop(Display *dpy, Window win, Atom atom, char *str, unsigned int size)
{
	char **list = NULL;
	int n;
	XTextProperty name;

	if(!str || size == 0)
		return 0;
	str[0] = '\0';
	XGetTextProperty(dpy, win, &name, atom);
	if(!name.nitems)
		return 0;
	if(name.encoding == XA_STRING)
		strncpy(str, (char *)name.value, size - 1);
	else {
		if(XmbTextPropertyToTextList(dpy, &name, &list, &n)
				>= Success && n > 0 && *list) {
			strncpy(str, *list, size - 1);
			XFreeStringList(list);
		}
	}
	str[size - 1] = '\0';
	XFree(name.value);
	return 1;

}
