#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define streq(s1, s2) (strcmp(s1, s2) == 0)

void die(const char *fmt, ...);

void error(const char *fmt, ...);

char *strfvs(char **, char);

void *xcalloc(size_t, size_t);

void *xmalloc(size_t);

char *xstrdup(const char *);

#endif
