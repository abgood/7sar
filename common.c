#include "tsar.h"

int get_strtok_num(char *str, char *split) {
    int num = 0;
    char *token, n_str[LEN_4096] = {0};

    // str为空时,返回0
    if (!str || !strlen(str))
        return 0;

    memcpy(n_str, str, strlen(str));
    token = strtok(n_str, split);
    while (token) {
        num++;
        token = strtok(NULL, split);
    }

    return num;
}

int is_digit(char *str) {
    while (*str) {
        if (!isdigit(*str++))
            return 0;
    }
    return 1;
}

/* convert record to the type of ull store in mod->cur_array 
 * return: store in mod->cur_array numbers
 * 这个函数写的有问题,正确的见下面
int convert_record_to_array(U_64 *array, int l_array, char *record) {
    int i = 0;
    char n_str[LEN_4096] = {0};
    char *token;

    if (!record || !strlen(record))
        return 0;
    memcpy(n_str, record, strlen(record));

    token = strtok(n_str, DATA_SPLIT);
    while (token) {
        if (!is_digit(token))
            return 0;
        if (i < l_array)
            *(array + i) = strtoull(token, NULL, 10);
        i++;
        token = strtok(NULL, DATA_SPLIT);
    }
    if (i != l_array)
        return 0;

    return i;
}
*/

int convert_record_to_array(U_64 *array, int l_array, char *record) {
    int i = 0;
    char n_str[LEN_4096] = {0};
    char *token;

    if (!record || !strlen(record))
        return 0;
    memcpy(n_str, record, strlen(record));

    token = strtok(n_str, DATA_SPLIT);
    while (token) {
        if (is_digit(token) && i < l_array - 1)
            array[i++] = strtoull(token, NULL, 10);
        else
            return i;
        token = strtok(NULL, DATA_SPLIT);
    }
    return i;
}

/* mul-items merge one string */
int merge_one_string(U_64 *array, int l_array, char *string, struct module *mod, int n_item) {
    int i, len;
    U_64 array_2[MAX_COL_NUM] = {0};
    struct mod_info *info = mod->info;

    if (!(len = convert_record_to_array(array_2, l_array, string)))
        return 0;
    /* 假如io参数需要合并
     * sda 和 hdc
     * 累加两个盘的前6位记录
     * 平均值两个盘的后5位记录
     */
    for (i = 0; i < len; i++) {
        switch (info[i].merge_mode) {
            case MERGE_SUM:
                /* 求和 */
                array[i] += array_2[i];
                break;
            case MERGE_AVG:
                /* 求平均值 */
                array[i] = (array[i] * (n_item - 1) + array_2[i]) / n_item;
                break;
            default:
                ;
        }
    }

    return 1;
}

/* 把多项目内的记录每次copy至item里进行处理 */
int strtok_next_item(char item[], char *record, int *start) {
    char *s_token, *e_token, *n_record;

    if (!record || !strlen(record) || strlen(record) <= *start)
        return 0;

    n_record = record + *start;
    if(!(e_token = strstr(n_record, ITEM_SPLIT)))
        return 0;
    if (!(s_token = strstr(n_record, ITEM_SPSTART)))
        return 0;

    memcpy(item, s_token + sizeof(ITEM_SPSTART) - 1, e_token - s_token - 1);
    *start = e_token - record + sizeof(ITEM_SPLIT);
    return 1;
}

/* in merge mode, merge mul-items */
int merge_mult_item_to_array(U_64 *array, struct module *mod) {
    char item[LEN_128] = {0};
    int pos = 0;
    int n_item = 1;

    /* 分配合并模式下的内存空间 */
    memset(array, 0, sizeof(U_64) * mod->n_col);
    while (strtok_next_item(item, mod->record, &pos)) {
        if (!merge_one_string(array, mod->n_col, item, mod, n_item))
            return 0;
        n_item++;
        memset(&item, 0, sizeof(item));
    }

    return 1;
}

void get_mod_hdr(char hdr[], struct module *mod) {
    int i, pos = 0;
    struct mod_info *info = mod->info;

    for (i = 0; i < mod->n_col; i++) {
        if (mod->spec) {
            if (SPEC_BIT == info[i].summary_bit) {
                if (strlen(info[i].hdr) > 6)
                    info[i].hdr[6] = '\0';
                pos += sprintf(hdr + pos, "%s%s", info[i].hdr, PRINT_DATA_SPLIT);
            }
        } else if (((DATA_SUMMARY == conf.print_mode) && (SUMMARY_BIT == info[i].summary_bit))
                || ((DATA_DETAIL == conf.print_mode) && (HIDE_BIT != info[i].summary_bit))) {
            if (strlen(info[i].hdr) > 6)
                info[i].hdr[6] = '\0';
            pos += sprintf(hdr + pos, "%s%s", info[i].hdr, PRINT_DATA_SPLIT);
        }
    }
}

int get_st_array_from_file(int have_collect) {
    int i, ret = 0;
    char line[LEN_10240] = {0};
    char detail[LEN_1024] = {0};
    char pre_line[LEN_10240] = {0};
    struct module *mod;
    FILE *fp;
    char *s_token;
    char pre_time[32] = {0};

    /* 收集数据 */
    if (!have_collect)
        collect_record();

    /* 合并模式 */
    conf.print_merge = MERGE_ITEM;

    sprintf(line, "%ld", statis.cur_time);
    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (mod->enable && strlen(mod->record)) {
            memset(&detail, 0, sizeof(detail));
            sprintf(detail, "%s%s%s%s", SECTION_SPLIT, mod->opt_line, STRING_SPLIT, mod->record);
            strcat(line, detail);
        }
    }

    if (strlen(line))
        strcat(line, "\n");

    if ((fp = fopen(PRE_RECORD_FILE, "r"))) {
        /* 无内容 */
        if (!fgets(pre_line, LEN_10240, fp)) {
            fclose(fp);
            ret = -1;
            goto out;
        }
    } else {
        /* 无文件 */
        ret = -1;
        goto out;
    }

    s_token = strstr(pre_line, SECTION_SPLIT);
    if (!s_token) {
        ret = -1;
        goto out;
    }
    memcpy(pre_time, pre_line, s_token - pre_line);
    /* 时间相同 */
    if (!(conf.print_interval = statis.cur_time - atol(pre_time)))
        goto out;

    read_line_to_module_record(pre_line);
    init_module_fields();
    collect_record_stat();

    read_line_to_module_record(line);
    collect_record_stat();
    ret = 0;

out:
    if ((fp = fopen(PRE_RECORD_FILE, "w"))) {
        strcat(line, "\n");
        fputs(line, fp);
        fclose(fp);
        chmod(PRE_RECORD_FILE, 0666);
    }

    return ret;
}
