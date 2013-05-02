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
	int num = 0;
	char *token, n_str[LEN_4096] = {0};

	if (!str || !strlen(str))
		return 0;

	memcpy(n_str, str, strlen(str));
	/* set print opt line */
	token = strtok(n_str, split);
	while (token) {
		num++;
		token = strtok(NULL, split);
	}

	return num;
}

int strtok_next_item(char item[], char *record, int *start) {
    char *s_token, *e_token, *n_record;

    if (!record || !strlen(record) || strlen(record) <= *start)
        return 0;

    n_record = record + *start;
    if (!(e_token = strstr(n_record, ITEM_SPLIT)))
        return 0;
    if (!(s_token = strstr(n_record, ITEM_SPSTART)))
        return 0;
    memcpy(item, s_token + sizeof(ITEM_SPSTART) - 1, e_token - s_token - 1);
    *start = e_token - record + sizeof(ITEM_SPLIT);

    return 1;
}

int is_digit(char *str) {
    while (*str) {
        if (!isdigit(*str++))
            return 0;
    }
    return 1;
}

/* convert record to array */
int convert_record_to_array(U_64 *array, int l_array, char *record) {
    char *token;
    char n_str[LEN_4096] = {0};
    int i = 0;

    if (!record || !strlen(record))
        return 0;
    memcpy(n_str, record, strlen(record));

    token = strtok(n_str, DATA_SPLIT);
    while (token) {
        if (!is_digit(token))
            return 0;
        if (i < l_array)
            *(array + i) = strtoull(token, NULL, 10);
            //printf("%d\n", strtoull(token, NULL, 10));
            //printf("%d\n", array[i]);
        token = strtok(NULL, DATA_SPLIT);
        i++;
    }
    if (i != l_array)
        return 0;

    return i;
}

int merge_one_string(U_64 *array, int l_array, char *string, struct module *mod, int n_item) {
    int i, len;
    U_64 array_2[MAX_MOD_NUM] = {0};
    struct mod_info *info = mod->info;

    if (!(len = convert_record_to_array(array_2, l_array, string)))
        return 0;

    for (i = 0; i < len; i++) {
        switch(info[i].merge_mode) {
            case MERGE_SUM:
                array[i] += array_2[i];
                break;
            case MERGE_AVG:
                array[i] = (array[i] * (n_item - 1) + array_2[i]) / n_item;
                break;
            default: ;
        }
    }

    return 1;
}

int merge_mult_item_to_array(U_64 *array, struct module *mod) {
	char item[LEN_128] = {0};
	int pos = 0;
	int n_item = 1;

	memset(array, 0, sizeof(U_64) * mod->n_col);
	while (strtok_next_item(item, mod->record, &pos)) {
		if(!merge_one_string(array, mod->n_col, item, mod, n_item))
			return 0;
		n_item++;
		memset(&item, 0, sizeof(item));
	}
	return 1;
}
