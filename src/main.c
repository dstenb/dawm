#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "config.h"
#include "utils.h"
#include "version.h"
#include "wm.h"

static void usage(const char *);
static void version(void);

/* prints an usage message */
void usage(const char *cmd)
{
	version();
	printf("\nUsage: %s [OPTION]...\n\n", cmd);
	printf("  -h, --help         print this message.\n");
	printf("  -v, --version      print version message.\n");
}

/* prints a version message */
void version(void)
{
	printf("wm: %s\n", VERSION);
}

int main(int argc, char **argv)
{
	struct config *cfg;
	struct wm *wm;
	char *cfg_str = NULL;
	int i;

	for (i = 1; i < argc; i++) {
		if (STREQ(argv[i], "-v") || STREQ(argv[i], "--version")) {
			version();
			exit(0);
		} else if (STREQ(argv[i], "--display") ||
				STREQ(argv[i], "-d")) {
			if (++i == argc)
				die("missing argument for %s\n", argv[i - 1]);
			setenv("DISPLAY", argv[i], 1);
		} else if (STREQ(argv[i], "--config") ||
				STREQ(argv[i], "-c")) {
			if (++i == argc)
				die("missing argument for %s\n", argv[i - 1]);
			cfg_str = argv[i];
		} else {
			usage(argv[0]);
			exit(1);
		}
	}

	cfg = config_init();

	wm = wm_init();
	wm_eventloop(wm);

	if (wm->restart)
		execlp("/bin/sh", "sh" , "-c", strfvs(argv, ' '), NULL);

	wm_destroy(wm);

	return 0;
}
