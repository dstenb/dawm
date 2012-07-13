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

static int check_config = 0;

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
	printf("  -k, --check        check if the given config file is ok.\n");
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

	for (i = 1; i < argc; i++) {
		if (STREQ(argv[i], "-v") || STREQ(argv[i], "--version")) {
			version();
			exit(EXIT_SUCCESS);
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
		} else if (STREQ(argv[i], "--check") ||
				STREQ(argv[i], "-k")) {
			check_config = 1;
		} else {
			usage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	cfg_str = cfg_str ? cfg_str : config_default_path();
	cfg = config_create();
	if (config_load(cfg, cfg_str) != 0)
		error("error loading '%s': %s\n", cfg_str, strerror(errno));

	if (check_config) {
		printf("%s: configuration file ok!\n", WMNAME);
		exit(EXIT_SUCCESS);
	}

	error("starting!\n");

	wm = init(cfg, strfvs(argv, ' '));
	eventloop(wm);
	destroy(wm);

	return 0;
}
