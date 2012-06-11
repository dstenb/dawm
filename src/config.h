#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "errno.h"
#include "keys.h"
#include "utils.h"

#define SIZE_COL_ARR 3

typedef enum {
	FG,
	BG,
	BORDER
} ConfigColorID;

struct config {
	int topbar;
	int showbar;
	int bsize;
	struct key *keys;

	/* normal window colors */
	char *col_win_norm[3];
	/* selected window colors */
	char *col_win_sel[3];
};

/* init a config struct, will set all the settings to the default values */
struct config *config_init(void);

/* load settings from a file */
int config_load(struct config *, const char *);

/* returns the default config path */
char *config_default_path(void);

void config_free(struct config *);

#endif
