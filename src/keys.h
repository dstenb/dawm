#ifndef _KEYS_H_
#define _KEYS_H_

#include <stdlib.h>
#include <stdio.h>

#include <X11/keysym.h>
#include <X11/Xlib.h>

#include "utils.h"

#define MOD_ALT   Mod1Mask
#define MOD_CTRL  ControlMask
#define MOD_SHIFT ShiftMask
#define MOD_SUPER Mod4Mask

#define MOD_CTRL_SUPER  (MOD_CTRL | MOD_SUPER)
#define MOD_SHIFT_SUPER (MOD_SHIFT | MOD_SUPER)

typedef enum {
	KillAction,
	MoveWindowAction,
	QuitAction,
	RestartAction,
	SetLayoutAction,
	SetTagAction,
	SpawnAction,
	ToggleBarAction,
	ToggleFloatAction,
	LASTAction,
	InvalidAction = -1
} KeyAction;

struct key {
	unsigned int mod;
	KeySym keysym;
	KeyAction action;
	char *args;
	struct key *next;
};

const char *key_action2str(KeyAction);

/* returns the KeyAction responding to the key, returns INVALID if not found */
KeyAction key_action_from_str(const char *);

/* appends two key lists to one, will return the head pointer */
struct key *key_append(struct key *, struct key *);

/* copies a key struct */
struct key *key_copy(struct key *);

/* creates a new key struct */
struct key *key_create(unsigned int, KeySym, KeyAction, char *, struct key *);

/* returns a key list of the default bindings */
struct key *key_default_keys(void);

/* recursively frees a key list. returns a NULL pointer for convenience */
struct key *key_free_all(struct key *);

/* grab all keys */
void key_grab_all(struct key *, Display *, Window);

/* returns non-zero if the key is pressed */
int key_pressed(struct key *, Display *, KeyCode, unsigned int);

/* returns non-zero if the string describes a modifier */
int key_str2mod(const char *);

/* update the Num Lock modifier */
void key_init(Display *);

#endif
