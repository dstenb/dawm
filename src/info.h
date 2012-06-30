#ifndef _INFO_H_
#define _INFO_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utils.h"

#ifdef __linux__
typedef enum {
	Unknown,
	Charging,
	Discharging,
	Full
} BatteryStatus;
#endif /* __linux__ */

struct info {
	time_t time;    /* current time */
#ifdef __linux__
	int cpu;        /* cpu usage, -1 if unable to calculate cpu */
	int batlevel;   /* battery level, -1 if unable to get value */
	int batstatus;  /* battery status */
	long uptime;    /* uptime in seconds, -1 if unable to get value */
	long mem_used;  /* used memory (in kb), -1 if unable to get value */
	long mem_total; /* total memory (in kb), -1 if unable to get value */
#endif /* __linux__ */
};

/* returns a pointer to the main info struct */
const struct info *info(void);

/* initializes the main info struct */
void info_init(void);

/* updates the main info struct */
void info_update(void);

#endif
