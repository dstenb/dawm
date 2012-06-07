#include "config.h"

static struct key *parse_bind(const char *, int, char *, char *, char *);

struct config *config_init(void)
{
	struct config *cfg = xcalloc(1, sizeof(struct config));

	cfg->keys = key_default_keys();
	cfg->topbar = 1;
	cfg->showbar = 1;
	cfg->bsize = 1;

	return cfg;
}

struct key *parse_bind(const char *path, int line, char *keystr,
		char *keyaction, char *args)
{
	unsigned int modifier = 0;
	KeySym keysym = NoSymbol;
	KeyAction action;
	char *p;

	if (!keystr)
		die("%s:%i, missing key binding\n", path, line);
	if (!keyaction)
		die("%s:%i, missing key action\n", path, line);

	for (p = strtok(keystr, "+"); p; p = strtok(NULL, "+")) {
		if (str_to_modifier(p) != -1) {
			modifier |= str_to_modifier(p);
		} else if (XStringToKeysym(p) != NoSymbol) {
			if (keysym != NoSymbol)
				die("%s:%i, two keys given\n", path, line);
			keysym = XStringToKeysym(p);
		} else {
			die("%s:%i, unknown key '%s'\n", path, line, p);
		}
	}

	if ((action = key_action_from_str(keyaction)) == InvalidAction)
		die("%s:%i, invalid action '%s'\n", path, line, keyaction);

	return key_create(modifier, keysym, action, args, NULL);
}

int config_load(struct config *cfg, const char *path)
{
	FILE *fp;
	char buf[4096];
	int line = 0;
	char *cmd;

	if (!(fp = fopen(path, "r")))
		return errno;

	while (fgets(buf, sizeof(buf), fp)) {
		line++;

		cmd = strtok(buf, " \t\n");

		if (!cmd || *cmd == '#' || *cmd == '\0') {
			continue;
		} else if (STREQ(cmd, "unbind_all")) {
			cfg->keys = key_free_all(cfg->keys);
		} else if (STREQ(cmd, "bind")) {
			char *keystr = strtok(NULL, " \t\n");
			char *actionstr = strtok(NULL, " \t\n");
			char *args = strtok(NULL, "\n");

			struct key *key = parse_bind(path, line, keystr,
					actionstr, args);
			cfg->keys = key_append(cfg->keys, key);
		} else {
			die("%s:%i, unknown command '%s'\n", path, line, cmd);
		}
	}

	fclose(fp);

	return 0;
}

char *config_default_path(void)
{
	char buf[PATH_MAX + 1];
	
	snprintf(buf, sizeof(buf), "%s/.config/wmrc", getenv("HOME"));

	return xstrdup(buf);
}

void config_free(struct config *cfg)
{
	key_free_all(cfg->keys);
	free(cfg);
}
