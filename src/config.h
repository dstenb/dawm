#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "errno.h"
#include "keys.h"
#include "utils.h"

struct config {
	struct key *keys;
};

struct config *config_init(void);

int config_load(struct config *, const char *);

char *config_default_path(void);

#endif
