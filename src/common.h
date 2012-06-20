#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdlib.h>
#include <stdio.h>

#define XINERAMA

#define WMNAME "dawm"

#define MIN_TAG 0
#define MAX_TAG 8
#define N_TAGS (MAX_TAG - MIN_TAG + 1)

#define VALID_TAG(T) (T >= MIN_TAG && T <= MAX_TAG)

#define M_FACT 0.55
#define N_MASTER 1

#endif
