#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdlib.h>
#include <stdio.h>

#define WMNAME "dawm"

#define MIN_WS 0
#define MAX_WS 8
#define N_WORKSPACES (MAX_WS - MIN_WS + 1)
#define ALL_WS 0xFFFFFFFF

#define VALID_WORKSPACE(W) ((W >= MIN_WS && W <= MAX_WS) || W == ALL_WS)

#define M_FACT 0.55
#define M_FACTSTEP 0.05
#define N_MASTER 1

#define BAR_FONT "-*-profont-*-*-*-*-10-*-*-*-*-*-*-*"
#define BAR_UPDATE_RATE 5

#endif
