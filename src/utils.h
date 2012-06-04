#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define streq(s1, s2) (strcmp(s1, s2) == 0)

char *strfvs(char **, char);

#endif
