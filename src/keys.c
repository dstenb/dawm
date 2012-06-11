#include "keys.h"

/* default keys */
static struct key default_keys[] = {
	{ MOD_SHIFT_SUPER, XK_q,      QuitAction,    NULL,    NULL },
	{ MOD_CTRL_SUPER,  XK_r,      RestartAction, NULL,    NULL },
	{ MOD_SUPER,       XK_Return, SpawnAction,   "urxvt", NULL }
};

/* strings corresponding to KeyAction values */
static char *action_str[LASTAction] = {
	[KillAction] = "kill",
	[QuitAction] = "quit",
	[RestartAction] = "restart",
	[SpawnAction] = "spawn"
};

KeyAction key_action_from_str(const char *str)
{
	int i;

	for (i = 0; i < ARRSIZE(action_str); i++) {
		if (action_str[i] && STREQ(action_str[i], str))
			return i;
	}

	return InvalidAction;
}

struct key *key_append(struct key *klist, struct key *new)
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

struct key *key_copy(struct key *key)
{
	return key_create(key->mod, key->keysym, key->action, key->args, NULL);
}

struct key *key_create(unsigned int mod, KeySym keysym, KeyAction action,
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

struct key *key_default_keys(void)
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

struct key *key_free_all(struct key *key)
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

void key_grab_all(struct key *key, Display *dpy, Window root)
{
	for ( ; key; key = key->next) {
		XGrabKey(dpy, XKeysymToKeycode(dpy, key->keysym), key->mod,
				root, True, GrabModeAsync, GrabModeAsync);
	}

}

int str_to_modifier(const char *str)
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
