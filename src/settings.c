#include "settings.h"

void parse_bar(config_t *);
void parse_colors(config_t *);
void parse_key_binding(config_setting_t *);
void parse_key_bindings_list(config_setting_t *);
void parse_key_bindings_settings(config_setting_t *);
void parse_key_bindings(config_t *);
void parse_rules(config_t *);
void parse_workspaces(config_t *);
void replace_str(char **, char *);

static struct settings _settings;

void
parse_bar(config_t *cfg)
{
	config_setting_t *group;
	const char *str;
	int b;

	if (!(group = config_lookup(cfg, "bar")))
		return;
	if (!config_setting_is_group(group))
		die("bar: not a group\n", index);

	if (config_setting_lookup_bool(group, "show_bar", &b))
		_settings.showbar = b;
	if (config_setting_lookup_bool(group, "top_bar", &b))
		_settings.topbar = b;
	if (config_setting_lookup_string(group, "font", &str))
		replace_str(&_settings.barfont, xstrdup(str));
}

void
parse_colors(config_t *cfg)
{
	/* TODO */
}

/* TODO: cleanup */
void
parse_key_binding(config_setting_t *elem)
{
	const char *keystr = NULL;
	const char *actionstr = NULL;
	const char *args = NULL;
	char *keystr_cpy, *p;

	unsigned int modifier = 0;
	KeySym keysym = NoSymbol;
	KeyAction action;
	int index = config_setting_index(elem);

	if (!config_setting_is_group(elem))
		die("binding %i: not a group\n", index);
	if (!config_setting_lookup_string(elem, "keys", &keystr))
		die("binding %i: missing 'keys'\n", index);
	if (!config_setting_lookup_string(elem, "action", &actionstr))
		die("binding %i: missing 'action'\n", index);
	config_setting_lookup_string(elem, "args", &args);

	keystr_cpy = xstrdup(keystr);

	for (p = strtok(keystr_cpy, "+"); p; p = strtok(NULL, "+")) {
		if (key_str2mod(p) != -1) {
			modifier |= key_str2mod(p);
		} else if (XStringToKeysym(p) != NoSymbol) {
			if (keysym != NoSymbol)
				die("binding %i, two keys given\n", index);
			keysym = XStringToKeysym(p);
		} else {
			die("binding %i, unknown key '%s'\n", index, p);
		}
	}

	free(keystr_cpy);

	if ((action = key_str2action(actionstr)) == InvalidAction)
		die("binding %i: invalid action '%s'\n", index, actionstr);

	_settings.keys = key_append(key_create(modifier, keysym, action, args,
				NULL), _settings.keys);
}

void
parse_key_bindings_list(config_setting_t *bindings)
{
	config_setting_t *list;
	int i;

	if (!(list = config_setting_get_member(bindings, "list")))
		return;
	if (!config_setting_is_list(list))
		die("bindings.list not a list\n");

	for (i = 0; i < config_setting_length(list); i++)
		parse_key_binding(config_setting_get_elem(list, i));
}

void
parse_key_bindings_settings(config_setting_t *bindings)
{
	int b;

	if (config_setting_lookup_bool(bindings, "unbind_all_default", &b)) {
		if (b)
			_settings.keys = key_free_all(_settings.keys);
	}
}

void
parse_key_bindings(config_t *cfg)
{
	config_setting_t *bindings;

	if (!(bindings = config_lookup(cfg, "bindings")))
		return;
	if (!config_setting_is_group(bindings))
		die("bindings must be a group\n");

	parse_key_bindings_settings(bindings);
	parse_key_bindings_list(bindings);
}

void
parse_rules(config_t *cfg)
{
	/* TODO */
}

void
parse_workspaces(config_t *cfg)
{
	/* TODO */
}

const struct settings *settings()
{
	return &_settings;
}

void
settings_init()
{
	_settings.topbar = 1;
	_settings.showbar = 1;
	_settings.bw = 1;
	_settings.keys = key_default_keys();
	_settings.mfact = M_FACT;
	_settings.nmaster = N_MASTER;
	_settings.barfont = xstrdup(BAR_FONT);
}

void
settings_free()
{
	int i;

	for (i = 0; i < LASTColor; i++)
		free(_settings.colors[i]);
	key_free_all(_settings.keys);
	free(_settings.barfont);
}

int
settings_read(const char *path)
{
	FILE *fp;
	config_t cfg;

	if (!(fp = fopen(path, "r")))
		return errno;

	config_init(&cfg);

	if (!config_read(&cfg, fp)) {
		die("%s:%i: %s\n", path, config_error_line(&cfg),
				config_error_text(&cfg));
	}

	parse_bar(&cfg);
	parse_colors(&cfg);
	parse_key_bindings(&cfg);
	parse_rules(&cfg);
	parse_workspaces(&cfg);

	config_destroy(&cfg);
	fclose(fp);

	return 0;
}

void
replace_str(char **p, char *new)
{
	if (*p)
		free(*p);
	*p = new;
}
