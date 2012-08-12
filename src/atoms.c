#include "dawm.h"

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

/** Get the atom with the given ID */
Atom
atom(AtomID id)
{
	assert(id < LASTAtom);
	return atoms[id];
}

/** Initializes all atoms */
void
atoms_init(void)
{
	error("%s\n", __func__);
	XInternAtoms(dpy, atom_names, ARRSIZE(atom_names), 0, atoms);
}

/** Append a Window to the given Window list property */
void
atom_append_window(Window win, Atom prop, Window propwin)
{
	XChangeProperty(dpy, win, prop, XA_WINDOW, 32, PropModeAppend,
			(unsigned char *) &(propwin), 1);
}

/** Delete a property */
void
atom_delete(Window win, Atom prop)
{
	XDeleteProperty(dpy, win, prop);
}

/** Get a atom value from the given property */
int
atom_get_atom(Window win, Atom prop, Atom *value)
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

/** Get a list of atom values from the given property. The given list should
 * be freed with XFree() */
int
atom_get_atoms(Window win, Atom prop, Atom **values, unsigned *n_values)
{
	Atom a;
	int f;
	unsigned long n, r;
	unsigned char *p;

	if (XGetWindowProperty(dpy, win, prop, 0L, 0x7FFFFFFF, False, XA_ATOM,
				&a, &f, &n, &r, &p) == Success && p) {
		*values = (Atom *) p;
		*n_values = n;
		return 1;
	}

	return 0;
}

/** Get a list of cardinal values from the given property. The given list should
 * be freed with XFree() */
int
atom_get_cardinals(Window win, Atom prop, unsigned long **values, unsigned *n)
{
	Atom a;
	int f;
	unsigned long _n, r;
	unsigned char *p;

	if (XGetWindowProperty(dpy, win, prop, 0L, 0x7FFFFFFF, False,
				XA_CARDINAL, &a, &f, &_n, &r, &p) == Success
			&& p) {
		*values = (Atom *) p;
		*n = _n;
		return 1;
	}

	return 0;

}

/** Get a string value from the given property */
int
atom_get_string(Window win, Atom prop, char *str, unsigned int size)
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

/** Get a cardinal value from the given property */
int
atom_get_cardinal(Window win, Atom prop, unsigned long *value)
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

/** Set the given property to the given atom value */
void
atom_set_atom(Window win, Atom prop, Atom value)
{
	XChangeProperty(dpy, win, prop, XA_ATOM, 32, PropModeReplace,
			(unsigned char *) &value, 1);
}

/** Set the given property to the list of the given atom value */
void
atom_set_atoms(Window win, Atom prop, Atom *props, unsigned n)
{
	XChangeProperty(dpy, win, prop, XA_ATOM, 32, PropModeReplace,
			(unsigned char *) props, n);
}

/** Set the given property to the given cardinal value */
void
atom_set_cardinal(Window win, Atom prop, unsigned long value)
{
	XChangeProperty(dpy, win, prop, XA_CARDINAL, 32, PropModeReplace,
			(unsigned char *) &value, 1);
}

/** Set the given property to the given string value */
void
atom_set_string(Window win, Atom prop, char *str)
{
	XChangeProperty(dpy, win, prop, XA_STRING, 8, PropModeReplace,
			(unsigned char *)str, strlen(str));
}

/** Set the given property to the given UTF-8 array */
void
atom_set_utf8array(Window win, Atom prop,
		unsigned char *buf, unsigned size)
{
	XChangeProperty(dpy, win, prop, atom(UTF8String),
			8, PropModeReplace, buf, size);
}

/** Set the given property to the given Window value */
void
atom_set_window(Window win, Atom prop, Window propwin)
{
	XChangeProperty(dpy, win, prop, XA_WINDOW, 32, PropModeReplace,
			(unsigned char *) &(propwin), 1);
}
