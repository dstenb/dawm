#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "errno.h"
#include "keys.h"
#include "utils.h"

struct config {
	int topbar;
	int showbar;
	int bsize;
	struct key *keys;
};

/* init a config struct, will set all the settings to the default values */
struct config *config_init(void);

/* load settings from a file */
int config_load(struct config *, const char *);

/* returns the default config path */
char *config_default_path(void);

void config_free(struct config *);

#endif
