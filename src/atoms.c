#include "atoms.h"

static char *atom_names[LASTAtom] = {
	"WM_CHANGE_STATE",
	"WM_DELETE_WINDOW",
	"WM_NAME",
	"WM_PROTOCOLS",
	"WM_STATE",
	"WM_TAKE_FOCUS",
	"UTF8_STRING"
};

static Atom atoms[LASTAtom];

Atom
atom(AtomID id)
{
	assert(id < LASTAtom);
	return atoms[id];
}

void
atoms_init(Display *dpy)
{
	error("%s\n", __func__);
	XInternAtoms(dpy, atom_names, ARRSIZE(atom_names), 0, atoms);
}

int has_wm_protocol(Display *dpy, Window win, Atom prot)
{
	int n;
	int found = 0;
	Atom *protocols;

	if (XGetWMProtocols(dpy, win, &protocols, &n)) {
		while (!found && n--)
			found = protocols[n] == prot;
		XFree(protocols);
	}

	return found;
}

void
atom_append_window(Display *dpy, Window win, Atom prop, Window propwin)
{
	XChangeProperty(dpy, win, prop, XA_WINDOW, 32, PropModeAppend,
			(unsigned char *) &(propwin), 1);
}

void
atom_delete(Display *dpy, Window win, Atom prop)
{
	XDeleteProperty(dpy, win, prop);
}

int
atom_get_atom(Display *dpy, Window win, Atom prop, Atom *value)
{
	Atom a;
	int f;
	unsigned long n, r;
	unsigned char *p;

	if (XGetWindowProperty(dpy, win, prop, 0L, sizeof(Atom), False,
				XA_ATOM, &a, &f, &n, &r, &p) == Success && p) {
		*value = *(Atom *) p;
		XFree(p);
		return 1;
	}

	return 0;
}

int
atom_get_string(Display *dpy, Window win, Atom prop,
		char *str, unsigned int size)
{
	char **list = NULL;
	int n;
	XTextProperty name;

	if(!str || size == 0)
		return 0;
	str[0] = '\0';
	XGetTextProperty(dpy, win, &name, prop);
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

int
atom_get_cardinal(Display *dpy, Window win, Atom prop, unsigned long *value)
{
	Atom a;
	int f;
	unsigned long n, r;
	unsigned char *p;

	if (XGetWindowProperty(dpy, win, prop, 0, 1L, False, XA_CARDINAL, &a,
				&f, &n, &r, &p) == Success && p) {
		*value = *(unsigned long *) p;
		XFree(p);
		return 1;
	}

	return 0;
}

void
atom_set_atoms(Display *dpy, Window win, Atom prop, Atom *props, unsigned n)
{
	XChangeProperty(dpy, win, prop, XA_ATOM, 32, PropModeReplace,
			(unsigned char *) props, n);
}

void
atom_set_cardinal(Display *dpy, Window win, Atom prop, unsigned long value)
{
	XChangeProperty(dpy, win, prop, XA_CARDINAL, 32, PropModeReplace,
			(unsigned char *) &value, 1);
}

void
atom_set_string(Display *dpy, Window win, Atom prop, char *str)
{
	XChangeProperty(dpy, win, prop, XA_STRING, 8, PropModeReplace,
			(unsigned char *)str, strlen(str));
}

void
atom_set_utf8array(Display *dpy, Window win, Atom prop,
		unsigned char *buf, unsigned size)
{
	XChangeProperty(dpy, win, prop, atom(UTF8String),
			8, PropModeReplace, buf, size);

}

void
atom_set_window(Display *dpy, Window win, Atom prop, Window propwin)
{
	XChangeProperty(dpy, win, prop, XA_WINDOW, 32, PropModeReplace,
			(unsigned char *) &(propwin), 1);

}
