#include "tsar.h"

void output_nagios(void) {
    int now_time;
    char host_name[LEN_64] = {0};
    int i = 0, j = 0, k = 0, l = 0, result = 0;
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

    conf.print_merge = MERGE_NOT;
    if (get_st_array_from_file(0))
        return;

    reload_modules(conf.output_nagios_mod);

    sprintf(s_time, "%ld", time(NULL));

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->enable)
            continue;
        else if (!mod->st_flag)
            printf("name %s do nothing\n", mod->name);
        else {
            char opt[LEN_32];
            char check[LEN_64];
            char *n_record = strdup(mod->record);
            char *token = strtok(n_record, ITEM_SPLIT);
            char *s_token;
            double *st_array;
            struct mod_info *info = mod->info;
            j = 0;

            while (token) {
                memset(check, 0, sizeof(check));
                strcat(check, mod->name + 4);
                strcat(check, ".");
                s_token = strstr(token, ITEM_SPSTART);
                /* multi items */
                if (s_token) {
                    memset(opt, 0, sizeof(opt));
                    strncat(opt, token, s_token - token);
                    strcat(check, opt);
                    strcat(check, ".");
                }
                /* get value */
                st_array = &mod->st_array[j * mod->n_col];
                token = strtok(NULL, ITEM_SPLIT);
                j++;

                for (k = 0; k < mod->n_col; k++) {
                    char check_item[LEN_64];
                    char *p;
                    memset(check_item, 0, LEN_64);
                    memcpy(check_item, check, LEN_64);
                    p = info[k].hdr;
                    while (*p == ' ')
                        p++;
                    strcat(check_item, p);
                    for (l = 0; l < conf.mod_num; l++) {
                        if (!strcmp(conf.check_name[l], check_item)) {
                            char value[LEN_32];
                            memset(value, 0, sizeof(value));
                            sprintf(value, "%0.2f", st_array[k]);
                            strcat(output, check_item);
                            strcat(output, "=");
                            strcat(output, value);
                            strcat(output, " ");

                            if (conf.cmin[l] != 0 && st_array[k] >= conf.cmin[l]) {
                            }

                            if (conf.wmin[l] != 0 && st_array[k] >= conf.wmin[l]) {
                            }
                        }
                    }
                }
            }
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
