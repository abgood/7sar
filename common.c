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

/* convert record to mod->cur_array */
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
        token = strtok(NULL, DATA_SPLIT);
        i++;
    }
    if (i != l_array)
        return 0;

    return i;
}

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
        printf("%s\n", item);
    }
}
