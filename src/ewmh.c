#include "dawm.h"

/* root window properties
 * http://standards.freedesktop.org/wm-spec/wm-spec-latest.html#id2533796
 */

/* application window properties
 * http://standards.freedesktop.org/wm-spec/wm-spec-latest.html#id2577833
 */

static char *atom_names[LASTNetAtom] = {
	"_NET_ACTIVE_WINDOW",
	"_NET_CLIENT_LIST",
	"_NET_CURRENT_DESKTOP",
	"_NET_WM_DESKTOP",
	"_NET_NUMBER_OF_DESKTOPS",
	"_NET_DESKTOP_NAMES",
	"_NET_SUPPORTING_WM_CHECK",
	"_NET_SUPPORTED",
	"_NET_WM_STATE_FULLSCREEN",
	"_NET_WM_NAME",
	"_NET_WM_STATE",
	"_NET_WM_STRUT",
	"_NET_WM_STRUT_PARTIAL",
	"_NET_WM_WINDOW_TYPE",
	"_NET_WM_WINDOW_TYPE_DESKTOP",
	"_NET_WM_WINDOW_TYPE_DIALOG",
	"_NET_WM_WINDOW_TYPE_DOCK"
};

static Atom netatoms[LASTNetAtom];
static const unsigned n_netatoms = ARRSIZE(netatoms);

/** Child window for _NET_SUPPORTING_WM_CHECK
 * http://standards.freedesktop.org/wm-spec/wm-spec-latest.html#id2577320 */
static Window child_win;

Atom
netatom(NetAtomID id)
{
	assert(id < LASTNetAtom);
	return netatoms[id];
}

void
ewmh_init(char *name)
{
	XSetWindowAttributes attr = {
		.override_redirect = True,
		.event_mask = 0
	};

	error("%s\n", __func__);

	/* init atoms */
	XInternAtoms(dpy, atom_names, ARRSIZE(atom_names), 0, netatoms);

	/* set supported atoms */
	atom_set_atoms(root, netatom(NetSupported), netatoms, n_netatoms);

	child_win = XCreateWindow(dpy, root, -10, -10, 1, 1, 0,
			DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen), 0, &attr);

	atom_set_window(root, netatom(NetSupportingCheck), child_win);
	atom_set_window(child_win, netatom(NetSupportingCheck), child_win);

	ewmh_root_set_name(name);
}

void
ewmh_root_client_list_add(Window win)
{
	atom_append_window(root, netatom(NetClientList), win);
}

void
ewmh_root_client_list_clear(void)
{
	atom_delete(root, netatom(NetClientList));
}

void
ewmh_root_set_active_window(Window win)
{
	atom_set_window(root, netatom(NetActiveWindow), win);
}

void
ewmh_root_set_current_desktop(unsigned long n)
{
	atom_set_cardinal(root, netatom(NetCurrentDesktop), n);
}

void
ewmh_root_set_name(char *name)
{
	atom_set_string(root, netatom(NetWMName), name);
	atom_set_string(child_win, netatom(NetWMName), name);
}

void
ewmh_root_set_number_of_desktops(unsigned long n)
{
	atom_set_cardinal(root, netatom(NetDesktops), n);
}

void
ewmh_root_set_desktop_names(unsigned char *names, unsigned n)
{
	atom_set_utf8array(root, netatom(NetDesktopNames), names, n);
}

bool
ewmh_client_get_desktop(Window win, unsigned long *d)
{
	return atom_get_cardinal(win, netatom(NetDesktop), d);
}

bool
ewmh_client_get_state(Window win, Atom *state)
{
	return atom_get_atom(win, netatom(NetWMState), state);
}

bool
ewmh_client_get_strut(Window win, struct strut_data *sd)
{
	unsigned long *values = NULL;
	unsigned n;
	bool ret = false;

	if (atom_get_cardinals(win, netatom(NetWMStrut), &values, &n)) {
		if (n == 4) {
			sd->left = values[0];
			sd->right = values[1];
			sd->top = values[2];
			sd->bottom = values[3];
			ret = true;
		}

		if (values)
			XFree(values);
	}

	return ret;
}

bool
ewmh_client_get_strut_partial(Window win, struct strut_data *sd)
{
	unsigned long *values = NULL;
	unsigned n;
	bool ret = false;

	if (atom_get_cardinals(win, netatom(NetWMStrutPartial),
				&values, &n)) {
		if (n == 12) {
			sd->left = values[0];
			sd->right = values[1];
			sd->top = values[2];
			sd->bottom = values[3];
			sd->left_start_y = values[4];
			sd->left_end_y = values[5];
			sd->right_start_y = values[6];
			sd->right_end_y = values[7];
			sd->top_start_x = values[8];
			sd->top_end_x = values[9];
			sd->bottom_start_x = values[10];
			sd->bottom_end_x = values[11];
			ret = true;
		}

		if (values)
			XFree(values);
	}

	return ret;
}

bool
ewmh_client_get_window_types(Window win, Atom **t, unsigned *n)
{
	return atom_get_atoms(win, netatom(NetWMWindowType), t, n);
}

void
ewmh_client_set_desktop(Window win, unsigned long d)
{
	atom_set_cardinal(win, netatom(NetDesktop), d);
}

void
ewmh_client_set_state(Window win, Atom state)
{
	atom_set_atom(win, netatom(NetWMState), state);
}
