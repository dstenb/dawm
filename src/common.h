#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdlib.h>
#include <stdio.h>

#define XINERAMA

#define WMNAME "dawm"

#define MIN_WS 0
#define MAX_WS 8
#define N_WORKSPACES (MAX_WS - MIN_WS + 1)

#define VALID_WORKSPACE(W) (W >= MIN_WS && W <= MAX_WS)

#define M_FACT 0.55
#define N_MASTER 1

#define BAR_FONT "-*-terminus-medium-r-*-*-12-*-*-*-*-*-*-*"
#define BAR_UPDATE_RATE 5

#endif
