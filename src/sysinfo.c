#include "dawm.h"

#ifdef SYSINFO_EXTENDED
static int get_battery(int *, int *);
static int get_cpu_stats(long *, long *);
static int get_mem(long *, long *);
static int get_uptime(long *);
#endif /* SYSINFO_EXTENDED */

static struct sysinfo _sysinfo;

#ifdef SYSINFO_EXTENDED
static struct {
	long u, i;   /* used and idle values */
	long ou, oi; /* previous used and idle values */
} cpu;

static const char *bat_full = "/sys/class/power_supply/BAT0/charge_full";
static const char *bat_now = "/sys/class/power_supply/BAT0/charge_now";
static const char *bat_status = "/sys/class/power_supply/BAT0/status";
static const char *bat_sym[] = { "", "+", "-", "" };

static int
get_battery_full(float *full)
{
	FILE *fp;

	if (!(fp = fopen(bat_full, "r"))) {
		error("fopen(\"%s\"): %s\n", bat_full, strerror(errno));
		return errno;
	}

	if (fscanf(fp, "%f", full) != 1) {
		error("fscanf: %s\n", strerror(errno));
		fclose(fp);
		return errno;
	}

	fclose(fp);
	return 0;
}

static int
get_battery_now(float *now)
{
	FILE *fp;

	if (!(fp = fopen(bat_now, "r"))) {
		error("fopen(\"%s\"): %s\n", bat_now, strerror(errno));
		return errno;
	}

	if (fscanf(fp, "%f", now) != 1) {
		error("fscanf: %s\n", strerror(errno));
		fclose(fp);
		return errno;
	}

	fclose(fp);
	return 0;
}

static int
get_battery_status(int *status)
{
	FILE *fp;
	char buf[32];

	if (!(fp = fopen(bat_status, "r"))) {
		error("fopen(\"%s\"): %s\n", bat_status, strerror(errno));
		return errno;
	}

	if (!fgets(buf, sizeof(buf), fp)) {
		error("fgets: %s\n", strerror(errno));
		fclose(fp);
		return errno;
	}

	if (strncmp(buf, "Full", strlen("Full")) == 0)
		*status = Full;
	else if (strncmp(buf, "Discharging", strlen("Discharging")) == 0)
		*status = Discharging;
	else if (strncmp(buf, "Charging", strlen("Charging")) == 0)
		*status = Charging;
	else
		*status = Unknown;

	fclose(fp);
	return 0;
}

int
get_battery(int *level, int *status)
{
	float full = 1, now = 0;

	*level = -1;
	*status = Unknown;

	if (get_battery_full(&full) != 0)
		return errno;

	if (get_battery_now(&now) != 0)
		return errno;

	*level = (int)(100 * now / full);

	if (get_battery_status(status) != 0)
		return errno;

	return 0;
}

int
get_cpu_stats(long *used, long *idle)
{
	FILE *fp;
	char buf[256];
	long u, n, s; /* used, nice and system */

	if (!(fp = fopen("/proc/stat", "r"))) {
		error("fopen(\"/proc/stat\"): %s\n", strerror(errno));
		return errno;
	}

	if (!fgets(buf, sizeof(buf), fp)) {
		error("fgets: %s\n", strerror(errno));
		fclose(fp);
		return errno;
	}

	sscanf(buf, "%*s %ld%ld%ld%ld", &u, &n, &s, idle);
	*used = u + n + s;

	fclose(fp);
	return 0;
}

int
get_mem(long *used, long *total)
{
	FILE *fp;
	char buf[256];
	long kbt, kbf, kbb, kbc; /* total, free, buffers, cached */

	if (!(fp = fopen("/proc/meminfo", "r"))) {
		error("fopen(\"/proc/meminfo\"): %s\n", strerror(errno));
		return errno;
	}

	while ((fgets(buf, sizeof(buf), fp))) {
		if (strncmp(buf, "MemTotal:", strlen("MemTotal:")) == 0)
			sscanf(buf, "%*s %ld", &kbt);
		else if (strncmp(buf, "MemFree:", strlen("MemFree:")) == 0)
			sscanf(buf, "%*s %ld", &kbf);
		else if (strncmp(buf, "Buffers:", strlen("Buffers:")) == 0)
			sscanf(buf, "%*s %ld", &kbb);
		else if (strncmp(buf, "Cached:", strlen("Cached:")) == 0)
			sscanf(buf, "%*s %ld", &kbc);
	}

	fclose(fp);

	*total = kbt;
	*used = (kbt - kbf - kbb - kbc);

	return 0;
}

int
get_uptime(long *uptime)
{
	FILE *fp;

	if (!(fp = fopen("/proc/uptime", "r"))) {
		error("fopen(\"/proc/uptime\"): %s\n", strerror(errno));
		return errno;
	}

	*uptime = -1;

	if (fscanf(fp, "%ld", uptime) != 1) {
		error("fscanf: %s\n", strerror(errno));
		fclose(fp);
		return errno;
	}

	fclose(fp);

	return 0;


}
#endif /* SYSINFO_EXTENDED */

/** Returns a pointer to the main sysinfo struct */
const struct sysinfo *
sysinfo()
{
	return &_sysinfo;
}

/** */
void
sysinfo_format(const char *fmt, char *str, unsigned size,
		struct format_data *fd)
{
	char buf[256];
	char *p;
	char c;
	unsigned j = 0;
	unsigned l;

	if (!str || size == 0)
		return;

	p = str;

	while ((c = *fmt++)) {
		if (j >= size - 1)
			break;

		if (c == '%') {
			c = *fmt++;

			switch(c) {
			case 'L': /* Layout symbol */
				l = MIN(size - j, strlen(fd->layout));
				p = stpncpy(p, fd->layout, l);
				j += l;
				break;
			case 'W': /* Workspace name */
				l = MIN(size - j, strlen(fd->workspace));
				p = stpncpy(p, fd->workspace, l);
				j += l;
				break;
			case 'T': /* Time */
				strftime(buf, sizeof(buf), TIME_FMT,
						localtime(&_sysinfo.time));
				l = MIN(size - j, strlen(buf));
				p = stpncpy(p, buf, l);
				j += l;
				break;
#ifdef SYSINFO_EXTENDED
			case 'P': /* Processor */
				snprintf(buf, sizeof(buf), "%i%c",
						_sysinfo.cpu, '%');
				l = MIN(size - j, strlen(buf));
				p = stpncpy(p, buf, l);
				j += l;
				break;
			case 'M': /* Memory */
				snprintf(buf, sizeof(buf), "%ld/%ld",
						_sysinfo.mem_used / 1024,
						_sysinfo.mem_total / 1024);
				l = MIN(size - j, strlen(buf));
				p = stpncpy(p, buf, l);
				j += l;
				break;
			case 'U': /* Uptime */
				snprintf(buf, sizeof(buf),
					"%ld days, %ld:%02ld",
					(_sysinfo.uptime / (3600 * 24)),
					(_sysinfo.uptime / 3600) % 24,
					(_sysinfo.uptime / 60) % 60);
				l = MIN(size - j, strlen(buf));
				p = stpncpy(p, buf, l);
				j += l;
				break;
			case 'B': /* Battery level */
				snprintf(buf, sizeof(buf), "%s%i%c",
						bat_sym[_sysinfo.bat_status],
						_sysinfo.bat_level, '%');
				l = MIN(size - j, strlen(buf));
				p = stpncpy(p, buf, l);
				j += l;
				break;
#endif /* SYSINFO_EXTENDED */
			default: /* Not a format '%' char */
				snprintf(buf, sizeof(buf), "%c%c", '%', c);
				p = stpncpy(p, buf, 2);
				j += 2;
				break;
			}
		} else {
			*p++ = c;
			j++;
		}
	}

	str[MIN(j, size - 1)] = '\0';
}

/** Initializes the main sysinfo struct */
void
sysinfo_init()
{
#ifdef SYSINFO_EXTENDED
	get_cpu_stats(&cpu.ou, &cpu.oi); /* get initial cpu values */
#endif /* SYSINFO_EXTENDED */

	sysinfo_update();
}

/** Updates the main sysinfo struct */
void
sysinfo_update()
{
	_sysinfo.time = time(NULL);

#ifdef SYSINFO_EXTENDED
	/* update the battery value */
	get_battery(&_sysinfo.bat_level, &_sysinfo.bat_status);
	get_mem(&_sysinfo.mem_used, &_sysinfo.mem_total);
	get_uptime(&_sysinfo.uptime);

	/* update the cpu value */
	_sysinfo.cpu = -1;
	if (get_cpu_stats(&cpu.u, &cpu.i) == 0) {
		_sysinfo.cpu = 100 * (cpu.u - cpu.ou) /
			(double) ((cpu.u + cpu.i) - (cpu.ou + cpu.oi));
		cpu.ou = cpu.u;
		cpu.oi = cpu.i;
	}
#endif /* SYSINFO_EXTENDED */
}
