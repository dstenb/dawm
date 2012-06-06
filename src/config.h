#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "utils.h"

struct config {


};

struct config *config_init(void);

int config_load(struct config *, const char *);

int config_load_default(struct config *);

char *config_default_path(void);

#endif
