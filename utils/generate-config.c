#include <stdio.h>
#include <stdlib.h>

#include "settings.h"

#BOOLSTR(b) (b ? "true" : "false")

void
print_bar()
{
	printf("bar:\n");
	printf("{\n");
	printf("  show_bar = %s;\n", BOOLSTR(settings()->showbar));
	printf("  top_bar = %s;\n", BOOLSTR(settings()->topbar));
	printf("  font = \"%s\";\n", settings()->barfont);
	printf("}\n");
}

void
print_colors()
{
	int i;

	printf("colors:\n");
	printf("(\n");
	for (i = 0; i < LASTColor; i++)
		printf("  { name=\"%s\" value=\"%s\" }%s\n",
				color_id2str(i), settings()->colors[i],
				i < LASTColor - 1 ? "," : "");
	printf(")\n");
}

void
print_keys()
{
	struct key *k;
	char buf[512];
	char *p;

	printf("bindings:\n");
	printf("{\n");
	printf("  /* setting this to true will unbind all the default key"
		"bindings\n     (the bindings are described below) */\n");
	printf("  unbind_all_default = false;\n");
	printf("\n");
	printf("  list:\n");
	printf("  (\n");
	for (k = settings()->keys; k; k = k->next) {
		buf[0] = '\0';
		p = buf;

		if (k->mod & MOD_SUPER)
			p = stpcpy(p, "super+");
		if (k->mod & MOD_CTRL)
			p = stpcpy(p, "ctrl+");
		if (k->mod & MOD_ALT)
			p = stpcpy(p, "alt+");
		if (k->mod & MOD_SHIFT)
			p = stpcpy(p, "shift+");

		stpcpy(p, XKeysymToString(k->keysym));

		printf("    #{ keys=\"%s\" action=\"%s\"", buf,
				key_action2str(k->action));
		if (k->args)
			printf(" args=\"%s\"", k->args);
		printf(" }%s\n", k->next ? "," : "");
	}
	printf("  )\n");
	printf("}\n");
}

void
print_workspaces()
{
	printf("workspaces:\n");
	printf("{\n");
	printf("}\n");
}

int
main(int argc, char **argv)
{
	settings_init();

	printf("### Example configuration for DAWM\n\n");
	print_bar();
	printf("\n");
	print_colors();
	printf("\n");
	print_keys();

	return 0;
}
