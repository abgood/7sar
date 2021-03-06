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
        U_64 pre_array[], U_64 cur_array[], int inter) {
    U_64 pre_total, cur_total;
    int i, j;
    pre_total = cur_total = 0;

    for (i = 0; i < mod->n_col; i++) {
        /* 新收集到的数据不能小于前一次的数据 */
        if (cur_array[i] < pre_array[i]) {
            for (j = 0; j < 9; j++)
                st_array[j] = -1;
            return;
        }
        pre_total += pre_array[i];
        cur_total += cur_array[i];
    }

    /* 新数据较前一次数据小或者无变化 */
    if (cur_total <= pre_total)
        return;

    /* 比上一次数大则存至st_array */
    for (i = 0; i < 9; i++) {
        /* st_array[5] is util */
        if ((i != 5) && (cur_array[i] >= pre_array[i]))
            st_array[i] = (cur_array[i] - pre_array[i]) * 100.0 / (cur_total - pre_total);
    }

    /* util = user + sys + hirq + sirq + nice */
    st_array[5] = st_array[0] + st_array[1] + st_array[3] + st_array[4] + st_array[6];
}

void mod_register(struct module *mod) {
    register_mod_fileds(mod, "--cpu", cpu_usage, cpu_info, 9, read_cpu_stats, set_cpu_record);
}
