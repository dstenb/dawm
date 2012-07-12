#include <stdio.h>
#include <stdlib.h>

#include "config.h"

static struct config *cfg;
void
print_colors()
{
	int i;

	printf("### Colors\n");
	for (i = 0; i < LASTColor; i++)
		printf("#color %s %s\n", color_id2str(i), cfg->colors[i]);
}

void
print_keys()
{
	struct key *k;
	char buf[512];
	char *p;

	printf("### Key bindings\n");
	for (k = cfg->keys; k; k = k->next) {
		buf[0] = '\0';
		p = buf;

		printf("#bind ");
		if (k->mod & MOD_SUPER)
			p = stpcpy(p, "super+");
		if (k->mod & MOD_CTRL)
			p = stpcpy(p, "ctrl+");
		if (k->mod & MOD_ALT)
			p = stpcpy(p, "alt+");
		if (k->mod & MOD_SHIFT)
			p = stpcpy(p, "shift+");

		stpcpy(p, XKeysymToString(k->keysym));

		printf("%s %s %s\n", buf, key_action2str(k->action),
				k->args ? k->args : "");
	}
}

int
main(int argc, char **argv)
{
	cfg = config_create();

	printf("### Example configuration for DAWM\n\n");
	printf("#bar_font %s\n\n", BAR_FONT);
	print_colors();
	printf("\n# The following command will unbind all keys\n");
	printf("#unbind_all\n\n");
	print_keys();

	return 0;
}
