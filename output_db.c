#include "tsar.h"

/* 把监测数据写入mysql里 */
void output_db(void) {
    struct module *mod;
    char value[LEN_4096] = {0};
    char field[LEN_128] = {0};
    int i, ret = 0;
    char cmd[LEN_10240] = {0};
    char token[LEN_128] = {0};
    const char *sql;

    sprintf(value, "from_unixtime(%ld),", statis.cur_time);
    strcat(field, "time,");
    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (mod->enable && strlen(mod->record)) {
            /* 组合插入语句 */
            sprintf(token, "\'%s\',", mod->record);
            strcat(value, token);
            sprintf(token, "%s,", mod->opt_line + 2);
            strcat(field, token);
            ret = 1;
        }
    }

    /* 去掉字符串最后面的逗号 */
    field[strlen(field) - 1] = '\0';
    value[strlen(value) - 1] = '\0';
    
    /* 组合mysql语句 */
    sprintf(cmd, "INSERT INTO data_collect(%s) values (%s)", field, value);
    sql = cmd;

    /* 是否执行正确正确mysql */
    if (query_mysql(conf.output_db_addr, sql))
        exit(1);
}
