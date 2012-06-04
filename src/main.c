#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "utils.h"
#include "version.h"
#include "wm.h"

static void usage(const char *);
static void version(void);

void usage(const char *cmd)
{
	version();
	printf("\nUsage: %s [OPTION]...\n\n", cmd);
	printf("  -h, --help         print this message.\n");
	printf("  -v, --version      print version message.\n");
}

void version(void)
{
	printf("wm: %s\n", VERSION);
}

int main(int argc, char **argv)
{
	struct wm *wm;
	char *cmd_str;
	int i;

	cmd_str = strfvs(argv, ' ');
	printf("'%s'\n", cmd_str);

	for (i = 1; i < argc; i++) {
		if (streq(argv[i], "-v") || streq(argv[i], "--version")) {
			version();
			exit(0);
		} else {
			usage(argv[0]);
			exit(1);
		}
	}

	if ((wm = wm_init())) {
		wm_eventloop(wm);

		if (wm->restart)
			execlp("/bin/sh", "sh" , "-c", cmd_str, NULL);

		wm_destroy(wm);
	} else {
		fprintf(stderr, "OH noes\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}
