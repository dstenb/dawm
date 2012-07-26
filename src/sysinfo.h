#ifndef _INFO_H_
#define _INFO_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utils.h"

#define SYSINFO_EXTENDED __linux__

#ifdef SYSINFO_EXTENDED
typedef enum {
	Unknown,
	Charging,
	Discharging,
	Full
} BatteryStatus;
#endif /* SYSINFO_EXTENDED */

struct sysinfo {
	time_t time;    /* current time */
#ifdef SYSINFO_EXTENDED
	int cpu;        /* cpu usage, -1 if unable to calculate cpu */
	int bat_level;  /* battery level, -1 if unable to get value */
	int bat_status; /* battery status */
	long uptime;    /* uptime in seconds, -1 if unable to get value */
	long mem_used;  /* used memory (in kb), -1 if unable to get value */
	long mem_total; /* total memory (in kb), -1 if unable to get value */
#endif /* SYSINFO_EXTENDED */
};

const struct sysinfo *sysinfo(void);
void sysinfo_init(void);
void sysinfo_update(void);

#endif
