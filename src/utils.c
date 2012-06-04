#include "utils.h"

char *strfvs(char **v, char c)
{
	char *s = NULL;
	char *p = NULL;
	size_t l = 0;
	int i;

	if (v && *v) {
		l = 1 + strlen(v[0]);
		for (i = 1; v[i]; i++)
			l += strlen(v[1]) + 1;

		if (!(s = calloc(l, sizeof(char))))
			return NULL;

		p = s;
		for (i = 0; v[i]; i++) {
			p = stpcpy(p, v[i]);
			*p++ = c;
		}
		*(p - 1) = '\0';
	}

	return s;
}
