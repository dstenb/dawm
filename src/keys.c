#include "dawm.h"

#define CLEANMASK(mask) (mask & ~(num_lock | LockMask))

/* default keys */
static struct key default_keys[] = {
	{ MOD_SUPER,       XK_x,      Kill,          NULL,     NULL },
	{ MOD_SHIFT_SUPER, XK_q,      Quit,          NULL,     NULL },
	{ MOD_CTRL_SUPER,  XK_r,      Restart,       NULL,     NULL },
	{ MOD_SHIFT_SUPER, XK_Return, Spawn,         "urxvt",  NULL },
	{ MOD_SUPER,       XK_b,      ToggleBar,     NULL,     NULL },
	{ MOD_SUPER,       XK_t,      ToggleFloat,   NULL,     NULL },
	{ MOD_SUPER,       XK_f,      ToggleFs,      NULL,     NULL },

	{ MOD_SUPER,       XK_Return, SetMaster,     NULL,     NULL },
	{ MOD_SUPER,       XK_l,      SetMasterFact, "+",      NULL },
	{ MOD_SUPER,       XK_h,      SetMasterFact, "-",      NULL },
	{ MOD_SUPER,       XK_period, SetMasterNum,  "+",      NULL },
	{ MOD_SUPER,       XK_comma,  SetMasterNum,  "-",      NULL },

	{ MOD_SUPER,       XK_j,      Select,        "next",   NULL },
	{ MOD_SUPER,       XK_k,      Select,        "prev",   NULL },

	{ MOD_SHIFT_SUPER, XK_j,      Swap,          "next",   NULL },
	{ MOD_SHIFT_SUPER, XK_k,      Swap,          "prev",   NULL },

	{ MOD_SUPER,       XK_1,      SetWs,         "1",      NULL },
	{ MOD_SUPER,       XK_2,      SetWs,         "2",      NULL },
	{ MOD_SUPER,       XK_3,      SetWs,         "3",      NULL },
	{ MOD_SUPER,       XK_4,      SetWs,         "4",      NULL },
	{ MOD_SUPER,       XK_5,      SetWs,         "5",      NULL },
	{ MOD_SUPER,       XK_6,      SetWs,         "6",      NULL },
	{ MOD_SUPER,       XK_7,      SetWs,         "7",      NULL },
	{ MOD_SUPER,       XK_8,      SetWs,         "8",      NULL },
	{ MOD_SUPER,       XK_9,      SetWs,         "9",      NULL },
	{ MOD_SUPER,       XK_Left,   SetWs,         "prev",   NULL },
	{ MOD_SUPER,       XK_Right,  SetWs,         "next",   NULL },
	{ MOD_SUPER,       XK_g,      ToggleWs,      NULL,     NULL },

	{ MOD_SHIFT_SUPER, XK_1,      MoveWindow,    "1",      NULL },
	{ MOD_SHIFT_SUPER, XK_2,      MoveWindow,    "2",      NULL },
	{ MOD_SHIFT_SUPER, XK_3,      MoveWindow,    "3",      NULL },
	{ MOD_SHIFT_SUPER, XK_4,      MoveWindow,    "4",      NULL },
	{ MOD_SHIFT_SUPER, XK_5,      MoveWindow,    "5",      NULL },
	{ MOD_SHIFT_SUPER, XK_6,      MoveWindow,    "6",      NULL },
	{ MOD_SHIFT_SUPER, XK_7,      MoveWindow,    "7",      NULL },
	{ MOD_SHIFT_SUPER, XK_8,      MoveWindow,    "8",      NULL },
	{ MOD_SHIFT_SUPER, XK_9,      MoveWindow,    "9",      NULL },
	{ MOD_SHIFT_SUPER, XK_Left,   MoveWindow,    "prev",   NULL },
	{ MOD_SHIFT_SUPER, XK_Right,  MoveWindow,    "next",   NULL },

	{ MOD_SUPER,       XK_h,      SetLayout,     "horz",   NULL },
	{ MOD_SUPER,       XK_v,      SetLayout,     "vert",   NULL },
	{ MOD_SUPER,       XK_m,      SetLayout,     "max",    NULL },
	{ MOD_SUPER,       XK_space,  SetLayout,     "next",   NULL },
	{ MOD_SHIFT_SUPER, XK_space,  SetLayout,     "prev",   NULL },

	{ MOD_SUPER,       XK_r,      SetLauncher,   NULL,     NULL }
};

/* strings corresponding to KeyAction values */
static char *action_str[LASTAction] = {
	[Kill] = "kill",
	[MoveWindow] = "move_window",
	[Quit] = "quit",
	[Restart] = "restart",
	[Select] = "select",
	[SetLauncher] = "launcher",
	[SetLayout] = "set_layout",
	[SetMaster] = "sel_to_master",
	[SetMasterFact] = "set_mfact",
	[SetMasterNum] = "set_n_masters",
	[SetWs] = "set_ws",
	[Spawn] = "spawn",
	[Swap] = "swap",
	[ToggleBar] = "toggle_bar",
	[ToggleFloat] = "toggle_float",
	[ToggleFs] = "toggle_fs",
	[ToggleWs] = "toggle_ws"
};

static unsigned int num_lock = 0;

/** converts a KeyAction to a string, returns NULL if the action is invalid */
const char *
key_action2str(KeyAction a)
{
	return (a >= 0 && a < LASTAction) ? action_str[a] : NULL;
}


/** appends two key lists to one, will return the head pointer */
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

/** copies a key struct */
struct key *
key_copy(struct key *key)
{
	return key_create(key->mod, key->keysym, key->action, key->args, NULL);
}

/** creates a new key struct */
struct key *
key_create(unsigned int mod, KeySym keysym, KeyAction action,
		const char *args, struct key *next)
{
	struct key *key = xmalloc(sizeof(struct key));

	key->mod =  mod;
	key->keysym = keysym;
	key->action = action;
	key->args = args ? xstrdup(args) : NULL;
	key->next = next;

	return key;
}

/** returns a key list of the default bindings */
struct key *
key_default_keys(void)
{
	struct key *tmp, *head = NULL;
	int i;

	for (i = ARRSIZE(default_keys) - 1; i >= 0; i--) {
		tmp = key_copy(&default_keys[i]);
		tmp->next = head;
		head = tmp;
	}

	return head;
}

/** recursively frees a key list. returns a NULL pointer for convenience */
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

/** grab all keys */
void
key_grab_all(struct key *key)
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

/** update the Num Lock modifier */
void
key_init(void)
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

/** returns non-zero if the key is pressed */
bool
key_pressed(struct key *key, KeyCode code, unsigned int state)
{
	return ((XKeysymToKeycode(dpy, key->keysym) == code) &&
			CLEANMASK(state) == key->mod);
}

/** returns the KeyAction responding to the key,
 * returns InvalidAction if not found */
KeyAction
key_str2action(const char *str)
{
	int i;

	for (i = 0; i < ARRSIZE(action_str); i++) {
		if (action_str[i] && STREQ(action_str[i], str))
			return i;
	}

	return InvalidAction;
}

/** returns non-zero if the string describes a modifier */
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
