#include "config.h"

static struct key *parse_bind(const char *, int, char *, char *, char *);
static void parse_color(struct config *, const char *, int, char *, char *);
static void parse_font(struct config *, const char *, int, char *);
static void replace_str(char **, char *);

static char default_path[PATH_MAX + 1];

static const char *default_colors[LASTColor] = {
	[BarBorder] = "#FF0000",
	[BarNormFG] = "#DDDDDD",
	[BarNormBG] = "#000000",
	[BarSelFG] = "#000000",
	[BarSelBG] = "#0000FF",
	[WinNormBorder] = "#000000",
	[WinSelBorder] = "#FF0000"
};

struct config *
config_create(void)
{
	struct config *cfg = xcalloc(1, sizeof(struct config));
	int i;

	cfg->keys = key_default_keys();
	cfg->topbar = 1;
	cfg->showbar = 1;
	cfg->bw = 1;
	cfg->nmaster = N_MASTER;
	cfg->mfact = M_FACT;
	cfg->barfont = xstrdup(BAR_FONT);

	for (i = 0; i < LASTColor; i++)
		cfg->colors[i] = xstrdup(default_colors[i]);

	return cfg;
}

struct key *
parse_bind(const char *path, int line, char *keystr,
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
		if (key_str2mod(p) != -1) {
			modifier |= key_str2mod(p);
		} else if (XStringToKeysym(p) != NoSymbol) {
			if (keysym != NoSymbol)
				die("%s:%i, two keys given\n", path, line);
			keysym = XStringToKeysym(p);
		} else {
			die("%s:%i, unknown key '%s'\n", path, line, p);
		}
	}

	if ((action = key_str2action(keyaction)) == InvalidAction)
		die("%s:%i, invalid action '%s'\n", path, line, keyaction);

	return key_create(modifier, keysym, action, args, NULL);
}

void
parse_color(struct config *cfg, const char *path, int line,
		char *cid, char *cvalue)
{
	if (!cid)
		die("%s:%i, missing color id\n", path, line);
	if (!cvalue)
		die("%s:%i, missing color value\n", path, line);
	if (color_str2id(cid) == InvalidColor)
		die("%s:%i, invalid color id: '%s'\n", path, line, cid);

	replace_str(&cfg->colors[color_str2id(cid)], xstrdup(cvalue));
}

void
parse_font(struct config *cfg, const char *path, int line,
		char *font)
{
	if (font)
		font = strtr(font, " \t\n");
	if (!font || *font == '\0')
		die("%s:%i, missing font value\n", path, line);

	replace_str(&cfg->barfont, xstrdup(font));
}

int
config_load(struct config *cfg, const char *path)
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
			cfg->keys = key_append(key, cfg->keys);
		} else if (STREQ(cmd, "color")) {
			char *cid = strtok(NULL, " \t\n");
			char *cvalue = strtok(NULL, " \t\n");

			parse_color(cfg, path, line, cid, cvalue);
		} else if (STREQ(cmd, "bar_font")) {
			parse_font(cfg, path, line, strtok(NULL, "\t\n"));
		} else {
			die("%s:%i, unknown command '%s'\n", path, line, cmd);
		}
	}

	fclose(fp);

	return 0;
}

char *
config_default_path(void)
{
	snprintf(default_path, sizeof(default_path),
			"%s/.config/dawm/config", getenv("HOME"));
	return default_path;
}

void
config_free(struct config *cfg)
{
	int i;

	for (i = 0; i < LASTColor; i++)
		free(cfg->colors[i]);

	key_free_all(cfg->keys);
	free(cfg->barfont);
	free(cfg);
}

void
replace_str(char **p, char *new)
{
	if (*p)
		free(*p);
	*p = new;
}
