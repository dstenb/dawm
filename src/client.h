#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdlib.h>
#include <stdio.h>

#include "common.h"

struct client {
	struct rect cur_r; /* current position and size */
	struct rect old_r; /* old position and size */

	int floating; /* non-zero if floating */
	int fullscreen; /* non-zero if fullscreen */

	struct client *next;
};

#endif
