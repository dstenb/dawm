#include "utils.h"

void
die(const char *fmt, ...)
{
	va_list val;

	fprintf(stderr, "%s: ", WMNAME);
	va_start(val, fmt);
	vfprintf(stderr, fmt, val);
	va_end(val);
	exit(EXIT_FAILURE);
}

void
dbg(const char *file, int line, const char *fmt, ...)
{
	va_list val;

	fprintf(stderr, "%s:%i:  ", file, line);
	va_start(val, fmt);
	vfprintf(stderr, fmt, val);
	va_end(val);
}

void
error(const char *fmt, ...)
{
	va_list val;

	fprintf(stderr, "%s: ", WMNAME);
	va_start(val, fmt);
	vfprintf(stderr, fmt, val);
	va_end(val);
}

void
spawn(const char *cmd)
{
	if (cmd && fork() == 0) {
		execlp("/bin/sh", "sh" , "-c", cmd, NULL);
		exit(1);
	}
}

char *
strfvs(char **v, char c)
{
	char *s = NULL;
	char *p = NULL;
	size_t l = 0;
	int i;

	if (v && *v) {
		l = 1 + strlen(v[0]);
		for (i = 1; v[i]; i++)
			l += strlen(v[i]) + 1;

		s = xcalloc(l, sizeof(char));

		p = s;
		for (i = 0; v[i]; i++) {
			p = stpcpy(p, v[i]);
			*p++ = c;
		}
		*(p - 1) = '\0';
	}

	return s;
}

char *
strtr(char *s, const char *skip)
{
	return strtrb(strtrf(s, skip), skip);

}

char *
strtrb(char *s, const char *skip)
{
	char *p = s + strlen(s) - 1;

	while (p > s && strchr(skip, *p))
		p--;
	*(p + 1) = '\0';

	return s;
}

char *
strtrf(char *s, const char *skip)
{
	while (*s && strchr(skip, *s))
		s++;
	return s;
}

void *
xcalloc(size_t nmemb, size_t size)
{
	void *data;

	if (!(data = calloc(nmemb, size)))
		die("couldn't malloc %u bytes\n", nmemb * size);
	return data;
}

void *
xmalloc(size_t size)
{
	void *data;

	if (!(data = malloc(size)))
		die("couldn't malloc %u bytes\n", size);
	return data;
}

char *
xstrdup(const char *str)
{
	char *cpy;

	if (!(cpy = strdup(str)))
		die("couldn't malloc %u bytes\n", sizeof(char) * strlen(str));
	return cpy;
}
