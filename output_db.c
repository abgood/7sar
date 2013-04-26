#include "tsar.h"

/* 把监测数据写入mysql里 */
void output_db(void) {
    MYSQL *conn;
    struct module *mod;
    char line[LEN_10240] = {0};
    char detail[LEN_4096] = {0};
    char s_time[LEN_256] = {0};
    int i, ret = 0;

    /* 是否正常连接mysql */
    if ((conn = conn_mysql(conf.output_db_addr)) == NULL)
        exit(1);
    
    sprintf(s_time, "%ld", statis.cur_time);
    strcat(line, s_time);
    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (mod->enable && strlen(mod->record)) {
            sprintf(detail, "%s%s%s%s", SECTION_SPLIT, mod->opt_line, STRING_SPLIT, mod->record);
            strcat(line, detail);
            ret = 1;
        }
    }
    strcat(line, "\n");
    printf("%s", line);
}
