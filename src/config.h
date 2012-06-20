#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "colors.h"
#include "common.h"
#include "errno.h"
#include "keys.h"
#include "utils.h"

#define SIZE_COL_ARR 3

struct config {
	int topbar;
	int showbar;
	int bw;
	struct key *keys;

	float mfact;
	unsigned int nmaster;

	char *colors[LASTColor];
};

/* init a config struct, will set all the settings to the default values */
struct config *config_init(void);

/* load settings from a file */
int config_load(struct config *, const char *);

/* returns the default config path */
char *config_default_path(void);

void config_free(struct config *);

#endif
