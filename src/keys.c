#include "keys.h"

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
	struct key *head = key_create(MOD_SUPER, XK_q, QUIT, NULL, NULL);
	head = key_create(MOD_CTRL | MOD_SUPER, XK_r, RESTART, NULL, head);
	head = key_create(MOD_SUPER, XK_Return, SPAWN, "urxvt", head);

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
