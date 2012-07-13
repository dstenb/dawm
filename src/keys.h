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
	KillAction,          /* kill client */
	MoveWindowAction,    /* move client to another workspace */
	QuitAction,          /* close the window manager */
	RestartAction,       /* restart the window manager */
	SetLayoutAction,     /* set the layout for the current workspace */
	SelectAction,        /* select a client */
	SetMasterAction,     /* move the client to the master position */
	SetMasterFactAction, /* set the master size */
	SetMasterNumAction,  /* set the number of master clients */
	SetWsAction,         /* set the current workspace */
	SpawnAction,         /* spawn a program in the background */
	SwapAction,          /* swap windows */
	ToggleBarAction,     /* show/hide the bar on the current monitor */
	ToggleFloatAction,   /* float/arrange the selected client */
	LASTAction,
	InvalidAction = -1
} KeyAction;

struct key {
	unsigned int mod; /* modifier mask */
	KeySym keysym;    /* keysym (XK_) */
	KeyAction action; /* action */
	char *args;       /* action arguments (can be NULL) */
	struct key *next; /* next key in key list */
};

const char *key_action2str(KeyAction);
struct key *key_append(struct key *, struct key *);
struct key *key_copy(struct key *);
struct key *key_create(unsigned int, KeySym, KeyAction, char *, struct key *);
struct key *key_default_keys(void);
struct key *key_free_all(struct key *);
void key_grab_all(struct key *, Display *, Window);
void key_init(Display *);
int key_pressed(struct key *, Display *, KeyCode, unsigned int);

KeyAction key_str2action(const char *);
int key_str2mod(const char *);

#endif
