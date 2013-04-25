#include "../tsar.h"

static char *cpu_usage = "    --cpu               CPU share (user, system, interrupt, nice, & idle)";

static struct mod_info cpu_info[] = {
	{"  user", DETAIL_BIT,  0,  STATS_NULL},
	{"   sys", DETAIL_BIT,  0,  STATS_NULL},
	{"  wait", DETAIL_BIT,  0,  STATS_NULL},
	{"  hirq", DETAIL_BIT,  0,  STATS_NULL},
	{"  sirq", DETAIL_BIT,  0,  STATS_NULL},
	{"  util", SUMMARY_BIT,  0,  STATS_NULL},
	{"  nice", HIDE_BIT,  0,  STATS_NULL},
	{" steal", HIDE_BIT,  0,  STATS_NULL},
	{" guest", HIDE_BIT,  0,  STATS_NULL},
};

struct stats_cpu {
	unsigned long long cpu_user;
	unsigned long long cpu_nice;
	unsigned long long cpu_sys;
	unsigned long long cpu_idle;
	unsigned long long cpu_iowait;
	unsigned long long cpu_steal;
	unsigned long long cpu_hardirq;
	unsigned long long cpu_softirq;
	unsigned long long cpu_guest;
};

static void read_cpu_stats(struct module *mod) {
    char buf[LEN_4096] = {0};
    char line[LEN_4096];
    struct stats_cpu st_cpu;
    FILE *fp;

    memset(&st_cpu, 0, sizeof(struct stats_cpu));
    if ((fp = fopen(STAT, "r")) == NULL)
        return;
    while (fgets(line, LEN_4096, fp)) {
        /* 这里比较的4个字符,不要漏掉后面那个空格 */
        if (!strncmp(line, "cpu ", 4)) {
			sscanf(line+5, "%llu %llu %llu %llu %llu %llu %llu %llu %llu",
					&st_cpu.cpu_user,
					&st_cpu.cpu_nice,
					&st_cpu.cpu_sys,
					&st_cpu.cpu_idle,
					&st_cpu.cpu_iowait,
					&st_cpu.cpu_hardirq,
					&st_cpu.cpu_softirq,
					&st_cpu.cpu_steal,
					&st_cpu.cpu_guest);
        }
    }

	int pos = sprintf(buf, "%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu",
			/* the store order is not same as read procedure */
			st_cpu.cpu_user,
			st_cpu.cpu_sys,
			st_cpu.cpu_iowait,
			st_cpu.cpu_hardirq,
			st_cpu.cpu_softirq,
			st_cpu.cpu_idle,
			st_cpu.cpu_nice,
			st_cpu.cpu_steal,
			st_cpu.cpu_guest);
    buf[pos] = '\0';
    set_mod_record(mod, buf);
    fclose(fp);
    return;
}

static void set_cpu_record(struct module *mod, double st_array[],
        U_64 pre_total[], U_64 cur_total[], int inter) {
    printf("set_cpu_record\n");
}

void mod_register(struct module *mod) {
    register_mod_fileds(mod, "--cpu", cpu_usage, cpu_info, 9, read_cpu_stats, set_cpu_record);
}
