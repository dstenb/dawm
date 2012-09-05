#include "dawm.h"

static void program_add(const char *, bool);
static bool program_find(const char *);
static bool program_load_from_dir(const char *);
static void program_print(void);

static struct list *program_list[256];

static int
sort_func(void *a, void *b)
{
	return strcmp(a, b);
}

void
program_add(const char *p, bool sort)
{
	unsigned i = (unsigned)*p;

	program_list[i] = list_prepend(program_list[i], xstrdup(p));

	if (sort)
		program_list[i] = list_sort(program_list[i], sort_func);
}

bool
program_find(const char *p)
{
	struct list *l;

	for (l = program_list[(unsigned)*p]; l && !STREQ(l->data, p);
			l = l->next);

	return l != NULL;
}

bool
program_load_from_dir(const char *path)
{
	struct dirent *de;
	DIR *dir;

	if (!(dir = opendir(path)))
		return false;

	while ((de = readdir(dir))) {
		if (!program_find(de->d_name)) {
			error("%s: adding: %s\n", __func__, de->d_name);
			program_add(de->d_name, false);
		}
	}

	closedir(dir);

	return true;
}

void
program_list_from_prefix(const char *s, struct list **_list)
{
	struct list *tmp = program_list[(unsigned)*s];
	struct list *list = NULL;

	/* Find first program */
	for ( ; tmp != NULL && !STRPREFIX((char *)tmp->data, s);
			tmp = tmp->next);

	/* Add all programs */
	for ( ; tmp != NULL && STRPREFIX((char *)tmp->data, s);
			tmp = tmp->next)
		list = list_append(list, tmp->data);

	*_list = list;
}

void
program_init(const char *paths)
{
	char *buf = xstrdup(paths);
	char *path;
	int i;

	if (!paths)
		return;

	for (path = strtok(buf, ":"); path; path = strtok(NULL, ":")) {
		char rpath[PATH_MAX + 1];
		realpath(path, rpath);
		if (!program_load_from_dir(rpath))
			error("couldn't open %s: %s", rpath, strerror(errno));
	}

	for (i = 0; i < 256; i++)
		program_list[i] = list_sort(program_list[i], sort_func);

	program_print();

	free(buf);
}

void
program_print(void)
{
	struct list *t;
	int i;

	for (i = 0; i < 256; i++) {
		printf("%c %i\n", i, i);
		for (t = program_list[i]; t; t = t->next) {
			printf("%s->", (char *)t->data);
		}
		printf("\n");
	}
}
