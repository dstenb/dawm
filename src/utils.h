#ifndef _UTILS_H_
#define _UTILS_H_

#include <sys/types.h>

#include <regex.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

#define ARRSIZE(x) (int)(sizeof(x) / sizeof(*x))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define STREQ(s1, s2) (strcmp(s1, s2) == 0)
#define INSIDE(x, y, bx, by, bw, bh) ((x >= bx) && x <= (bx + bw) && \
		(y >= by) && y <= (by + bh))

#define DEBUG 1

#if DEBUG
#define DBG(...) dbg(__FILE__, __LINE__, __VA_ARGS__)
#else
#define DBG(...)
#endif

void dbg(const char *, int, const char *, ...);
void die(const char *, ...);
void error(const char *, ...);

void spawn(const char *);

char *strfvs(char **, char);
int strmatch(const char *, const char *);
char *strtr(char *, const char *);
char *strtrb(char *, const char *);
char *strtrf(char *, const char *);

void *xcalloc(size_t, size_t);
void *xmalloc(size_t);
void *xrealloc(void *, size_t);
char *xstrdup(const char *);

#endif
