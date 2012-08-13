/* dawm - tiling window manager
 *
 * License: See LICENSE file
 */

#include <sys/stat.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "version.h"
#include "dawm.h"

static void autorun(void);
static void compiled(void);
static void usage(const char *);
static void version(void);

/* If this is set to true, the window manager will be closed after
 * the config file have been read */
static bool check_config = false;

/** (Tries) to execute the autorun file */
void
autorun(void)
{
	struct stat st;
	char path[PATH_MAX];
	char *home;

	if (!(home = getenv("HOME")))
		return;

	snprintf(path, sizeof(path), "%s/.config/dawm/autorun", home);

	if (stat(path, &st) != 0)
		return;

	/* See if the file is executable before using spawn() to avoid
	 * unwanted error messages from the shell */
	if (S_ISREG(st.st_mode) && (st.st_mode & S_IXUSR))
		spawn(path);
}

/** Prints a list of compiled features */
void
compiled(void)
{
	printf("Compiled features:\n");
#ifdef XINERAMA
	printf("   Xinerama support\n");
#endif
#ifdef XFT
	printf("   Xft support\n");
#endif

}

/** Prints an usage message */
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

/** Prints a version message */
void
version(void)
{
	printf("%s: %s\n\n", WMNAME, VERSION);
	compiled();
}

int
main(int argc, char **argv)
{
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
			check_config = true;
		} else {
			usage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	autorun();

	settings_init();
	settings_read(cfg_str ? cfg_str : settings_default_path());

	if (check_config)
		die("configuration file ok!\n");

	error("starting!\n");

	init(strfvs(argv, ' '));
	eventloop();
	destroy();

	return 0;
}
