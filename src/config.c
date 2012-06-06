#include "config.h"

struct config *config_init(void)
{
	struct config *cfg = calloc(1, sizeof(struct config));

	return cfg;
}

int config_load(struct config *cfg, const char *path)
{

}

char *config_default_path(void)
{
	char buf[PATH_MAX + 1];
	char *str;

	snprintf(buf, sizeof(buf), "%s/.config/wm", getenv("HOME"));

	return xstrdup(buf);
}
