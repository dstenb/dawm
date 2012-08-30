#include "dawm.h"

static struct launcher launcher;

static struct {
	struct key backspace, enter, escape, left, right;
} keys = {
	.backspace = { .mod = 0, .keysym = XK_BackSpace },
	.enter = { .mod = 0, .keysym = XK_Return },
	.escape = { .mod = 0, .keysym = XK_Escape },
	.left = { .mod = 0, .keysym = XK_Left },
	.right = { .mod = 0, .keysym = XK_Right },
};

static void launcher_char_add(char);
static void launcher_char_del(void);
static void launcher_clear(void);
static bool launcher_handle_char(XKeyEvent *);
static void launcher_move_left(void);
static void launcher_move_right(void);
static void launcher_spawn(void);

const char *
launcher_buffer(void)
{
	return launcher.buf;
}

void
launcher_char_add(char c)
{
	error("%s\n", __func__);
	if (launcher.pos < (LAUNCHER_BUFSIZE - 1)) {
		launcher.buf[launcher.pos++] = c;
		launcher.buf[launcher.pos] = '\0';
	}
}

void
launcher_char_del(void)
{
	error("%s\n", __func__);
	if (launcher.pos > 0) {
		launcher.buf[launcher.pos--] = '\0';
	}
}

void
launcher_clear(void)
{
	error("%s\n", __func__);
	memset(launcher.buf, 0, LAUNCHER_BUFSIZE);
	launcher.pos = 0;
}

bool
launcher_handle_char(XKeyEvent *kev)
{
	error("%s\n", __func__);
	KeySym key;
	char text[8];

	if (XLookupString(kev, text, sizeof(text), &key, 0) == 1) {
		launcher_char_add(text[0]);
		return true;
	} else {
		return false;
	}
}

void
launcher_init(void)
{
	error("%s\n", __func__);

	XSetWindowAttributes attr = {
		.override_redirect = True,
		.event_mask = 0
	};

	launcher.win = XCreateWindow(dpy, root, -10, -10, 1, 1, 0,
			DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen), 0, &attr);
	launcher.active = false;
	launcher_clear();

	XMapWindow(dpy, launcher.win);
}

bool
launcher_keypress(XKeyEvent *kev)
{
	error("%s\n", __func__);
	bool handled = false;

	if (key_pressed(&keys.escape, kev->keycode, kev->state)) {
		launcher_ungrab();
		handled = true;
	} else if (key_pressed(&keys.enter, kev->keycode, kev->state)) {
		launcher_spawn();
		handled = true;
	} else if (key_pressed(&keys.left, kev->keycode, kev->state)) {
		launcher_move_left();
		handled = true;
	} else if (key_pressed(&keys.right, kev->keycode, kev->state)) {
		launcher_move_right();
		handled = true;
	} else if (key_pressed(&keys.backspace, kev->keycode, kev->state)) {
		launcher_char_del();
		handled = true;
	} else {
		handled = launcher_handle_char(kev);
	}

	return handled;
}

void
launcher_grab(void)
{
	int i;

	error("%s\n", __func__);

	for (i = 0; i < 100; i++) {
		if (XGrabKeyboard(dpy, launcher.win, False, GrabModeAsync,
				GrabModeAsync, CurrentTime) == Success) {
			launcher.active = true;
			return;
		}
		printf("i: %i\n", i);
		usleep(20);
	}
}

void
launcher_move_left(void)
{
	error("%s\n", __func__);
	/* TODO */
}

void
launcher_move_right(void)
{
	error("%s\n", __func__);
	/* TODO */
}

void
launcher_spawn(void)
{
	error("%s\n", __func__);
	spawn(launcher.buf);
	launcher_clear();
	launcher_ungrab();
}

void
launcher_ungrab(void)
{
	error("%s\n", __func__);
	XUngrabKeyboard(dpy, CurrentTime);
	launcher.active = false;
}

Window
launcher_window(void)
{
	return launcher.win;
}
