#include "config.h"

struct config *config_init(void)
{
	struct config *cfg;

	if (!(cfg = calloc(1, sizeof(struct config))))
		die("couldn't malloc\n");

	return cfg;
}
