#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "colors.h"
#include "common.h"
#include "errno.h"
#include "keys.h"
#include "utils.h"

struct config {
	int topbar;              /* non-zero if bars should be in the top */
	int showbar;             /* non-zero if bars should be shown */
	int bw;                  /* window border width */
	struct key *keys;        /* key bindings */
	float mfact;             /* master size factor [0, 1] */
	unsigned int nmaster;    /* number of master clients */
	char *barfont;           /* bar font */
	char *colors[LASTColor]; /* color values, in "#XXXXXX" form */
};

struct config *config_create(void);
char *config_default_path(void);
void config_free(struct config *);
int config_load(struct config *, const char *);

#endif
