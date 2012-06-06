#include "keys.h"

static char *action_str[LASTAction] = {
	[KillAction] = "kill",
	[QuitAction] = "quit",
	[RestartAction] = "restart",
	[SpawnAction] = "spawn"
};

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

struct key *key_default_bindings(void)
{
	struct key *head = key_create(MOD_SUPER, XK_q, QuitAction, NULL, NULL);
	head = key_create(MOD_CTRL | MOD_SUPER, XK_r, RestartAction, NULL, head);
	head = key_create(MOD_SUPER, XK_Return, SpawnAction, "urxvt", head);

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

void key_print(struct key *key)
{
	for ( ; key; key = key->next) {
		printf("modmask: %u, action: %u, args: %s, next: %p -->\n",
				key->mod, key->action, key->args,
				(void *)key->next);
	}
}

KeyAction key_action_from_str(const char *str)
{
	int i;

	for (i = 0; i < ARRSIZE(action_str); i++) {
		if (action_str[i] && STREQ(action_str[i], str))
			return i;
	}

	return InvalidAction;
}

int str_to_modifier(const char *str)
{
	if (strcasecmp(str, "alt") == 0)
		return MOD_ALT;
	else if (strcasecmp(str, "super") == 0)
		return MOD_SUPER;
	else if (strcasecmp(str, "shift") == 0)
		return MOD_SHIFT;
	else if (strcasecmp(str, "ctrl") == 0)
		return MOD_CTRL;
	return -1;
}
