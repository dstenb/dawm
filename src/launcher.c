#include "dawm.h"

static struct launcher launcher;

static struct {
	struct list *list;
	struct list *curr;
	int size;
} autocomplete;

static struct {
	struct key backspace, enter, escape, left, right, tab;
} keys = {
	.backspace = { .mod = 0, .keysym = XK_BackSpace },
	.enter = { .mod = 0, .keysym = XK_Return },
	.escape = { .mod = 0, .keysym = XK_Escape },
	.left = { .mod = 0, .keysym = XK_Left },
	.right = { .mod = 0, .keysym = XK_Right },
	.tab = { .mod = 0, .keysym = XK_Tab }
};

static void launcher_autocomplete(void);
static void launcher_autocomplete_clear(void);
static void launcher_autocomplete_next(void);
static void launcher_autocomplete_retrieve(void);
static void launcher_char_add(char);
static void launcher_char_del(void);
static void launcher_clear(void);
static bool launcher_handle_char(XKeyEvent *);
static void launcher_move_left(void);
static void launcher_move_right(void);
static void launcher_set_buffer(const char *);
static void launcher_spawn(void);

bool
launcher_activated(void)
{
	return launcher.active;
}

void
launcher_autocomplete(void)
{
	if (strwc(launcher.buf) <= 1) {
		if (autocomplete.list)
			launcher_autocomplete_next();
		else
			launcher_autocomplete_retrieve();
	} else {
		/* TODO: handle argument completition (paths) */
	}
}

void
launcher_autocomplete_clear(void)
{
	list_destroy(autocomplete.list, NULL);
	autocomplete.size = 0;
	autocomplete.list = NULL;
	autocomplete.curr = NULL;
}

void
launcher_autocomplete_next(void)
{
	if (autocomplete.curr) {
		if (autocomplete.curr->next)
			autocomplete.curr = autocomplete.curr->next;
		else
			autocomplete.curr = autocomplete.list;
		launcher_set_buffer(autocomplete.curr->data);
	}
}

void
launcher_autocomplete_retrieve(void)
{
	program_list_from_prefix(launcher.buf, &autocomplete.list);
	autocomplete.size = list_length(autocomplete.list);

	if (autocomplete.size > 0) {
		autocomplete.curr = autocomplete.list;
		launcher_set_buffer(autocomplete.curr->data);
	} else {
		autocomplete.curr = NULL;
	}
}

const char *
launcher_buffer(void)
{
	return launcher.buf;
}

void
launcher_char_add(char c)
{
	if (launcher.pos < (LAUNCHER_BUFSIZE - 1)) {
		launcher.buf[launcher.pos++] = c;
		launcher.buf[launcher.pos] = '\0';
	}
}

void
launcher_char_del(void)
{
	if (launcher.pos > 0) {
		launcher.buf[--launcher.pos] = '\0';
	}
}

void
launcher_clear(void)
{
	memset(launcher.buf, 0, LAUNCHER_BUFSIZE);
	launcher.pos = 0;
	launcher_autocomplete_clear();
}

bool
launcher_handle_char(XKeyEvent *kev)
{
	bool handled = false;
	char text[8];
	KeySym key;

	if (XLookupString(kev, text, sizeof(text), &key, 0) == 1) {
		launcher_char_add(text[0]);
		handled = true;
	}
	return handled;
}

void
launcher_init(void)
{
	launcher.active = false;
	launcher.win = None;
	launcher_clear();
}

bool
launcher_keypress(XKeyEvent *kev)
{
	bool handled = false;

	if (key_pressed(&keys.escape, kev->keycode, kev->state)) {
		launcher_ungrab();
		launcher_clear();
		handled = true;
	} else if (key_pressed(&keys.enter, kev->keycode, kev->state)) {
		launcher_spawn();
		launcher_autocomplete_clear();
		handled = true;
	} else if (key_pressed(&keys.left, kev->keycode, kev->state)) {
		launcher_move_left();
		handled = true;
	} else if (key_pressed(&keys.right, kev->keycode, kev->state)) {
		launcher_move_right();
		handled = true;
	} else if (key_pressed(&keys.backspace, kev->keycode, kev->state)) {
		launcher_char_del();
		launcher_autocomplete_clear();
		handled = true;
	} else if (key_pressed(&keys.tab, kev->keycode, kev->state)) {
		launcher_autocomplete();
		handled = true;
	} else {
		handled = launcher_handle_char(kev);
	}

	return handled;
}

void
launcher_grab(void)
{
	XSetWindowAttributes attr = {
		.override_redirect = True,
		.event_mask = 0
	};
	int i;

	launcher.win = XCreateWindow(dpy, root, -10, -10, 1, 1, 0,
			DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen), 0, &attr);
	XMapWindow(dpy, launcher.win);

	for (i = 0; i < 100; i++) {
		if (XGrabKeyboard(dpy, launcher.win, False, GrabModeAsync,
				GrabModeAsync, CurrentTime) == Success) {
			launcher.active = true;
			return;
		}
		usleep(20);
	}
}

void
launcher_move_left(void)
{
	/* TODO */
}

void
launcher_move_right(void)
{
	/* TODO */
}

void
launcher_set_buffer(const char *s)
{
	snprintf(launcher.buf, sizeof(launcher.buf), "%s", s);
	launcher.pos = strlen(launcher.buf);
}

void
launcher_spawn(void)
{
	spawn(launcher.buf);
	launcher_clear();
	launcher_ungrab();
}

void
launcher_ungrab(void)
{
	XUngrabKeyboard(dpy, CurrentTime);
	XUnmapWindow(dpy, launcher.win);
	XDestroyWindow(dpy, launcher.win);

	launcher.active = false;
	launcher.win = None;
}

Window
launcher_window(void)
{
	return launcher.win;
}
