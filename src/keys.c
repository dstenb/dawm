#include "keys.h"

#define CLEANMASK(mask) (mask & ~(num_lock | LockMask))

/* default keys */
static struct key default_keys[] = {
	{ MOD_SUPER,       XK_x,      KillAction,        NULL,    NULL },
	{ MOD_SHIFT_SUPER, XK_q,      QuitAction,        NULL,    NULL },
	{ MOD_CTRL_SUPER,  XK_r,      RestartAction,     NULL,    NULL },
	{ MOD_SUPER,       XK_Return, SpawnAction,       "urxvt", NULL },
	{ MOD_SUPER,       XK_b,      ToggleBarAction,   NULL,   NULL },
	{ MOD_SUPER,       XK_t,      ToggleFloatAction, NULL, NULL },

	{ MOD_SUPER,       XK_1,      SetTagAction,      "1",     NULL },
	{ MOD_SUPER,       XK_2,      SetTagAction,      "2",     NULL },
	{ MOD_SUPER,       XK_3,      SetTagAction,      "3",     NULL },
	{ MOD_SUPER,       XK_4,      SetTagAction,      "4",     NULL },
	{ MOD_SUPER,       XK_5,      SetTagAction,      "5",     NULL },
	{ MOD_SUPER,       XK_6,      SetTagAction,      "6",     NULL },
	{ MOD_SUPER,       XK_7,      SetTagAction,      "7",     NULL },
	{ MOD_SUPER,       XK_8,      SetTagAction,      "8",     NULL },
	{ MOD_SUPER,       XK_9,      SetTagAction,      "9",     NULL },

	{ MOD_SHIFT_SUPER, XK_1,      MoveWindowAction,  "1",     NULL },
	{ MOD_SHIFT_SUPER, XK_2,      MoveWindowAction,  "2",     NULL },
	{ MOD_SHIFT_SUPER, XK_3,      MoveWindowAction,  "3",     NULL },
	{ MOD_SHIFT_SUPER, XK_4,      MoveWindowAction,  "4",     NULL },
	{ MOD_SHIFT_SUPER, XK_5,      MoveWindowAction,  "5",     NULL },
	{ MOD_SHIFT_SUPER, XK_6,      MoveWindowAction,  "6",     NULL },
	{ MOD_SHIFT_SUPER, XK_7,      MoveWindowAction,  "7",     NULL },
	{ MOD_SHIFT_SUPER, XK_8,      MoveWindowAction,  "8",     NULL },
	{ MOD_SHIFT_SUPER, XK_9,      MoveWindowAction,  "9",     NULL },

	{ MOD_SUPER,       XK_h,      SetLayoutAction,   "horz",   NULL },
	{ MOD_SUPER,       XK_v,      SetLayoutAction,   "vert",   NULL },
	{ MOD_SUPER,       XK_m,      SetLayoutAction,   "matrix", NULL },
	{ MOD_SUPER,       XK_n,      SetLayoutAction,   "float",  NULL },
	{ MOD_SUPER,       XK_space,  SetLayoutAction,   "next",   NULL },
	{ MOD_SHIFT_SUPER, XK_space,  SetLayoutAction,   "prev",   NULL }
};

/* strings corresponding to KeyAction values */
static char *action_str[LASTAction] = {
	[KillAction] = "kill",
	[MoveWindowAction] = "move_window",
	[QuitAction] = "quit",
	[RestartAction] = "restart",
	[SetLayoutAction] = "set_layout",
	[SetTagAction] = "set_tag",
	[SpawnAction] = "spawn",
	[ToggleBarAction] = "toggle_bar",
	[ToggleFloatAction] = "toggle_float"
};

static unsigned int num_lock = 0;

KeyAction
key_action_from_str(const char *str)
{
	int i;

	for (i = 0; i < ARRSIZE(action_str); i++) {
		if (action_str[i] && STREQ(action_str[i], str))
			return i;
	}

	return InvalidAction;
}

struct key *
key_append(struct key *klist, struct key *new)
{
	struct key *trav;

	if (klist) {
		for (trav = klist; trav->next; trav = trav->next) ;
		trav->next = new;
		return klist;
	} else {
		return new;
	}
}

struct key *
key_copy(struct key *key)
{
	return key_create(key->mod, key->keysym, key->action, key->args, NULL);
}

struct key *
key_create(unsigned int mod, KeySym keysym, KeyAction action,
		char *args, struct key *next)
{
	struct key *key = xmalloc(sizeof(struct key));

	key->mod =  mod;
	key->keysym = keysym;
	key->action = action;
	key->args = args ? xstrdup(args) : NULL;
	key->next = next;

	return key;
}

struct key *
key_default_keys(void)
{
	struct key *tmp, *head = NULL;
	int i;

	for (i = 0; i < ARRSIZE(default_keys); i++) {
		tmp = key_copy(&default_keys[i]);
		tmp->next = head;
		head = tmp;
	}

	return head;
}

struct key *
key_free_all(struct key *key)
{
	struct key *tmp;

	while (key) {
		tmp = key;
		key = key->next;
		free(tmp->args);
		free(tmp);
	}

	return NULL;
}

void
key_grab_all(struct key *key, Display *dpy, Window root)
{
	int i;
	unsigned int mod[] = { 0, LockMask, num_lock, num_lock | LockMask };

	for ( ; key; key = key->next) {
		for (i = 0; i < ARRSIZE(mod); i++) {
			XGrabKey(dpy, XKeysymToKeycode(dpy, key->keysym),
					key->mod | mod[i], root, True,
					GrabModeAsync, GrabModeAsync);
		}
	}
}

int
key_pressed(struct key *key, Display *dpy, KeyCode code, unsigned int state)
{
	return ((XKeysymToKeycode(dpy, key->keysym) == code) &&
			CLEANMASK(state) == key->mod);
}

int
key_str2mod(const char *str)
{
	if (strcasecmp(str, "alt") == 0)
		return MOD_ALT;
	else if (strcasecmp(str, "ctrl") == 0)
		return MOD_CTRL;
	else if (strcasecmp(str, "shift") == 0)
		return MOD_SHIFT;
	else if (strcasecmp(str, "super") == 0)
		return MOD_SUPER;
	return -1;
}

void
key_init(Display *dpy)
{
	unsigned int i, j;
	XModifierKeymap *modmap;

	num_lock = 0;
	modmap = XGetModifierMapping(dpy);

	for (i = 0; i < 8; i++)
		for (j = 0; j < (unsigned) modmap->max_keypermod; j++)
			if (modmap->modifiermap[i * modmap->max_keypermod + j]
					== XKeysymToKeycode(dpy, XK_Num_Lock))
				num_lock = (1 << i);

	XFreeModifiermap(modmap);
}
