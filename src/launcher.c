#include "dawm.h"

static struct {
	struct key backspace, enter, escape, left, right;
} keys = {
	.backspace = { .mod = 0, .keysym = XK_BackSpace },
	.enter = { .mod = 0, .keysym = XK_Return },
	.escape = { .mod = 0, .keysym = XK_Escape },
	.left = { .mod = 0, .keysym = XK_Left },
	.right = { .mod = 0, .keysym = XK_Right },
};

static void launcher_char_add(struct launcher *, char);
static void launcher_char_del(struct launcher *);
static void launcher_clear(struct launcher *);
static bool launcher_handle_char(struct launcher *, XKeyEvent *);
static void launcher_move_left(struct launcher *);
static void launcher_move_right(struct launcher *);
static void launcher_spawn(struct launcher *);

void
launcher_char_add(struct launcher *launcher, char c)
{
	error("%s\n", __func__);
	if (launcher->pos < (LAUNCHER_BUFSIZE - 1)) {
		launcher->buf[launcher->pos++] = c;
		launcher->buf[launcher->pos] = '\0';
	}
}

void
launcher_char_del(struct launcher *launcher)
{
	error("%s\n", __func__);
	if (launcher->pos > 0) {
		launcher->buf[launcher->pos--] = '\0';
	}
}

void
launcher_clear(struct launcher *launcher)
{
	error("%s\n", __func__);
	memset(launcher->buf, 0, LAUNCHER_BUFSIZE);
	launcher->pos = 0;
}

bool
launcher_handle_char(struct launcher *launcher, XKeyEvent *kev)
{
	error("%s\n", __func__);
	KeySym key;
	char text[8];

	if (XLookupString(kev, text, sizeof(text), &key, 0) == 1) {
		launcher_char_add(launcher, text[0]);
		return true;
	} else {
		return false;
	}
}

void
launcher_init(struct launcher **_launcher)
{
	error("%s\n", __func__);
	struct launcher *launcher;

	XSetWindowAttributes attr = {
		.override_redirect = True,
		.event_mask = 0
	};

	launcher = xcalloc(1, sizeof(struct launcher));
	launcher->win = XCreateWindow(dpy, root, -10, -10, 1, 1, 0,
			DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen), 0, &attr);
	launcher->active = false;
	launcher_clear(launcher);

	XMapWindow(dpy, launcher->win);

	*_launcher = launcher;
}

bool
launcher_keypress(struct launcher *launcher, XKeyEvent *kev)
{
	error("%s\n", __func__);
	bool handled = false;

	if (key_pressed(&keys.escape, kev->keycode, kev->state)) {
		launcher_ungrab(launcher);
		handled = true;
	} else if (key_pressed(&keys.enter, kev->keycode, kev->state)) {
		launcher_spawn(launcher);
		handled = true;
	} else if (key_pressed(&keys.left, kev->keycode, kev->state)) {
		launcher_move_left(launcher);
		handled = true;
	} else if (key_pressed(&keys.right, kev->keycode, kev->state)) {
		launcher_move_right(launcher);
		handled = true;
	} else if (key_pressed(&keys.backspace, kev->keycode, kev->state)) {
		launcher_char_del(launcher);
		handled = true;
	} else {
		handled = launcher_handle_char(launcher, kev);
	}

	return handled;
}

void
launcher_grab(struct launcher *launcher)
{
	int i;

	error("%s\n", __func__);

	for (i = 0; i < 100; i++) {
		if (XGrabKeyboard(dpy, launcher->win, False, GrabModeAsync,
				GrabModeAsync, CurrentTime) == Success) {
			launcher->active = true;
			return;
		}
		printf("i: %i\n", i);
		usleep(20);
	}
}

void
launcher_move_left(struct launcher *launcher)
{
	error("%s\n", __func__);
	(void)launcher;
	/* TODO */
}

void
launcher_move_right(struct launcher *launcher)
{
	error("%s\n", __func__);
	(void)launcher;
	/* TODO */
}

void
launcher_spawn(struct launcher *launcher)
{
	error("%s\n", __func__);
	spawn(launcher->buf);
	launcher_clear(launcher);
	launcher_ungrab(launcher);
}

void
launcher_ungrab(struct launcher *launcher)
{
	error("%s\n", __func__);
	XUngrabKeyboard(dpy, CurrentTime);
	launcher->active = false;
}
