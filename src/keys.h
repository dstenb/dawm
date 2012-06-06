#ifndef _KEYS_H_
#define _KEYS_H_

#include <stdlib.h>
#include <stdio.h>

#include <X11/keysym.h>
#include <X11/Xlib.h>

#include "utils.h"

/* TODO: fix conflicts */

#define MOD_ALT Mod1Mask
#define MOD_CTRL ControlMask
#define MOD_SHIFT ShiftMask
#define MOD_SUPER Mod4Mask

typedef enum {
	SPAWN,
	KILL,
	QUIT,
	RESTART
} KeyAction;

struct key {
	unsigned int mod;
	KeySym keysym;
	KeyAction action;
	char *args;
	struct key *next;
};

/* creates a new key struct */
struct key *key_create(unsigned int, KeySym, KeyAction, char *, struct key *);

/* appends two key lists to one, will return the head pointer */
struct key *key_append(struct key *, struct key *);

/* returns a key list of the default bindings */
struct key *key_default_bindings(void);

/* recursively frees a key list. returns a NULL pointer for convenience */
struct key *key_free_all(struct key *);

/* print a key list, for development purposes */
void key_print(struct key *);

#endif
