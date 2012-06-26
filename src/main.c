#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "common.h"
#include "config.h"
#include "utils.h"
#include "version.h"
#include "wm.h"

static void usage(const char *);
static void version(void);

/* prints an usage message */
void
usage(const char *cmd)
{
	version();
	printf("\nUsage: %s [OPTION]...\n\n", cmd);
	printf("  -h, --help         print this message.\n");
	printf("  -v, --version      print version message.\n");
	printf("  -c, --config       select a custom configuration file.\n");
	printf("  -d, --display      select a custom display.\n");
}

/* prints a version message */
void
version(void)
{
	printf("%s: %s\n", WMNAME, VERSION);
}

int
main(int argc, char **argv)
{
	struct config *cfg;
	struct wm *wm;
	char *cfg_str = NULL;
	int i;

	error("starting!\n");

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

	cfg_str = cfg_str ? cfg_str : config_default_path();
	cfg = config_init();
	if (config_load(cfg, cfg_str) != 0)
		error("error loading '%s': %s\n", cfg_str, strerror(errno));

	wm = init(cfg, strfvs(argv, ' '));
	eventloop(wm);
	destroy(wm);

	return 0;
}
