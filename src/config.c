#include "config.h"

struct config *config_init(void)
{
	struct config *cfg = xcalloc(1, sizeof(struct config));

	cfg->keys = key_default_bindings();

	return cfg;
}

int config_load(struct config *cfg, const char *path)
{
	FILE *fp;
	char buf[4096];
	int i;
	char *cmd;

	if (!(fp = fopen(path, "r")))
		return errno;

	while (fgets(buf, sizeof(buf), fp)) {
		i++;

		cmd = strtok(buf, " \t\n");

		if (!cmd || *cmd == '#' || *cmd == '\0') {
			continue;
		} else if (STREQ(cmd, "unbind_all")) {
			cfg->keys = key_free_all(cfg->keys);
		} else if (STREQ(cmd, "bind")) {

		} else {
			die("%s:%i, unknown command '%s'\n", path, i, cmd);
		}
	}

	fclose(fp);

	return 0;
}

char *config_default_path(void)
{
	char buf[PATH_MAX + 1];
	char *str;

	snprintf(buf, sizeof(buf), "%s/.config/wmrc", getenv("HOME"));

	return xstrdup(buf);
}
