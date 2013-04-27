#include "tsar.h"

void output_nagios(void) {
    int now_time;
    char host_name[LEN_64] = {0};
    int i = 0;
    char s_time[LEN_64] = {0};
    struct module *mod;

    now_time = statis.cur_time - statis.cur_time%60;
    /* 轮转时间是否正常 */
    if ((*conf.cycle_time) == 0 || now_time%*(conf.cycle_time) != 0)
        return;

    if (0 != gethostname(host_name, sizeof(host_name)))
        do_debug(LOG_FATAL, "send to nagios: gethostname err, errno=%d \n", errno);
    /* 验证host_name的每个字符是不是可打印字符 */
    while(host_name[i]) {
        if (!isprint(host_name[i++])) {
            host_name[i - 1] = '\0';
            break;
        }
    }

    conf.print_merge = MERGE_NOT;
    /*
    if (get_st_array_from_file(0))
        return;
    */

    reload_modules(conf.output_nagios_mod);

    sprintf(s_time, "%ld", time(NULL));

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->enable)
            continue;
        else if (!mod->st_flag)
            printf("name %s do nothing\n", mod->name);
        else {
            char *n_record = strdup(mod->record);
            char *token = strtok(n_record, ITEM_SPLIT);
            printf("%s\n", token);
        }
    }
}
