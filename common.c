#include "tsar.h"

int get_st_array_from_file(int have_collect) {
    char line[10240] = {0};
    char pre_line[10240] = {0};
    int i, ret = 0;
    struct module *mod;
    char detail[LEN_1024] = {0};
    FILE *fp;
    char *s_token;
    char pre_time[LEN_32] = {0};

    if (!have_collect)
        collect_record();

    conf.print_merge = MERGE_ITEM;
    sprintf(line, "%ld", statis.cur_time);
    /* 循环读模板所收集的信息并统计 */
    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (mod->enable && strlen(mod->record)) {
            sprintf(detail, "%s%s%s%s", SECTION_SPLIT, mod->opt_line, STRING_SPLIT, mod->record);
            strcat(line, detail);
        }
    }
    if (strlen(line))
        strcat(line, "\n");

    if ((fp = fopen(PRE_RECORD_FILE, "r"))) {
        // 文件可读但没内容
        if (!fgets(pre_line, LEN_10240, fp)) {
            fclose(fp);
            ret = -1;
            goto out;
        }
    } else {
        ret = -1;
        goto out;
    }

    /* 所取内容是否所需 */
    s_token = strstr(pre_line, SECTION_SPLIT);
    if (!s_token) {
        ret = -1;
        goto out;
    }
    // 取时间字段
    memcpy(pre_time, pre_line, s_token - pre_line);
    if (!(conf.print_interval = statis.cur_time - atol(pre_time)))
        goto out;

    read_line_to_module_record(pre_line);
    init_module_fields();
    // collect_record_stat();

    read_line_to_module_record(line);
    //collect_record_stat();
    ret = 0;


    /* 写入记录到 /tmp/.tsar.tmp */
out:
    if ((fp = fopen(PRE_RECORD_FILE, "w"))) {
        strcat(line, "\n");
        fputs(line, fp);
        fclose(fp);
        chmod(PRE_RECORD_FILE, 0666);
    }

    return ret;
}

int get_strtok_num(char *str, char *split) {
    char *token, n_str[LEN_4096] = {0};
    int num = 0;
    
    if (!str || !strlen(str))
        return 0;

    memcpy(n_str, str, strlen(str));
    token = strtok(n_str, split);
    while (token) {
        num++;
        strtok(NULL, split);
    }

    return num;
}
