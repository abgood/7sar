#include "tsar.h"

void output_nagios(void) {
    int now_time;
    char host_name[LEN_64] = {0};
    int i = 0, result = 0;
    char s_time[LEN_64] = {0};
    struct module *mod;
    char output_err[LEN_4096] = {0};
    char output[LEN_4096] = {0};

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

    /*
    conf.print_merge = MERGE_NOT;
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

    /* 组合nagios命令串,把数据写到nagios服务器上 */
    if (!strcmp(output_err, ""))
        strcat(output_err, "OK");

    char nagios_cmd[LEN_1024];
    sprintf(nagios_cmd, "echo \"%s;tsar;%d;%s|%s\"|%s -H %s -p %d -to 10 -d \";\" -c %s", host_name, result, output_err, output, conf.send_nsca_cmd, conf.server_addr, *(conf.server_port), conf.send_nsca_conf);

    do_debug(LOG_DEBUG, "send to nagios:%s\n", nagios_cmd);

    /*
    if (system(nagios_cmd) != 0)
        do_debug(LOG_WARN, "nsca run error:%s\n", nagios_cmd);
    */

    printf("%s\n", nagios_cmd);
    printf("%d\n", conf.mod_num);
}
