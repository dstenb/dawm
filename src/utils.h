#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

#define STREQ(s1, s2) (strcmp(s1, s2) == 0)

#define ARRSIZE(x) (int)(sizeof(x) / sizeof(*x))

#define MAX(a, b) ((a) > (b) ? (a) : (b))

/* prints an error message and exits */
void die(const char *, ...);

/* prints an error message */
void error(const char *, ...);

/* spawn a process in the background */
void spawn(const char *);

/* concatenates a NULL-terminated list of strings to one */
char *strfvs(char **, char);

/* calloc() wrapper that will exit if unable to allocate */
void *xcalloc(size_t, size_t);

/* malloc() wrapper that will exit if unable to allocate */
void *xmalloc(size_t);

/* strdup() wrapper that will exit if unable to allocate */
char *xstrdup(const char *);

#endif
