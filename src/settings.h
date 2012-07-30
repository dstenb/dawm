#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <libconfig.h>

#include "colors.h"
#include "common.h"
#include "keys.h"
#include "rules.h"
#include "utils.h"

struct settings {
	bool topbar;             /* non-zero if bars should be in the top */
	bool showbar;            /* non-zero if bars should be shown */
	struct key *keys;        /* key bindings */
	float mfact;             /* master size factor [0, 1] */
	unsigned int nmaster;    /* number of master clients */
	char *barfont;           /* bar font */
	char *colors[LASTColor]; /* color values, in "#XXXXXX" form */
	int bw;
};

const struct settings *settings(void);
char *settings_default_path(void);
void settings_init(void);
void settings_free(void);
void settings_read(const char *);

#endif
