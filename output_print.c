#include "tsar.h"

char *trim(char *src, int max_len) {
    int cur_len = 0;
    char *index = src;
    while (*index == ' ' && cur_len < max_len) {
        index++;
        cur_len++;
    }
    return index;
}

void running_check(int check_type) {
    char host_name[LEN_64] = {0};
    char check[LEN_10240] = {0};
    char filename[LEN_128] = {0};
    char line[2][10240];
    int i, j, k, total_num = 0;
    FILE *fp;
    struct module *mod = NULL;
    double *st_array;
    char tmp[9][LEN_256];

    if (0 != gethostname(host_name, sizeof(host_name)))
        do_debug(LOG_FATAL, "tsar -check: gethostname err, errno=%d\n", errno);
    i = 0;
    while (host_name[i]) {
        if (!isprint(host_name[i++])) {
            host_name[i - 1] = '\0';
            break;
        }
    }
    sprintf(check, "%s\ttsar\t", host_name);
    sprintf(filename, "%s", conf.output_file_path);
    if (!(fp = fopen(filename, "r")))
        do_debug(LOG_FATAL, "Unable to open the log file %s\n", filename);
    memset(&line[0], 0, LEN_10240);
    total_num = 0;
    fseek(fp, -1, SEEK_END);
    while (1) {
        if (fgetc(fp) == '\n') ++total_num;
        /* 在tsar文件的倒数第三个'\n'上面break,文件指针停在倒数第三个'\n'上面 */
        if (total_num == 3) break;
        if (fseek(fp, -2, SEEK_CUR) != 0) {
            fseek(fp, 0, SEEK_SET);
            break;
        }
    }

    if (total_num == 0) {
        fclose(fp);
        memset(filename, 0, sizeof(filename));
        sprintf(filename, "%s.1", conf.output_file_path);
        if (!(fp = fopen(filename, "r")))
            do_debug(LOG_FATAL, "Unable to open the log file %s\n", filename);
        total_num = 0;
        memset(&line[0], 0, 2 * LEN_10240);
        fseek(fp, -1, SEEK_END);
        while (1) {
            if (fgetc(fp) == '\n') ++total_num;
            if (total_num == 3) break;
            if (fseek(fp, -2, SEEK_CUR) != 0) {
                fseek(fp, 0, SEEK_SET);
                break;
            }
        }
        if (total_num < 2)
            do_debug(LOG_FATAL, "not enough lines at log file %s.\n", filename);
        memset(&line[0], 0, LEN_10240);
        fgets(line[0], LEN_10240, fp);
        memset(&line[1], 0, LEN_10240);
        fgets(line[1], LEN_10240, fp);
    } else if (total_num == 1) {
        memset(&line[1], 0, LEN_10240);
        fgets(line[1], LEN_10240, fp);
        fclose(fp);
        sprintf(filename, "%s.1", conf.output_file_path);
        if (!(fp = fopen(filename, "r")))
            do_debug(LOG_FATAL, "Unable to open the log file %s\n", filename);
        total_num = 0;
        fseek(fp, -1, SEEK_END);
        while (1) {
            if (fgetc(fp) == '\n') ++total_num;
            if (total_num == 2) break;
            if (fseek(fp, -2, SEEK_CUR) != 0) {
                fseek(fp, 0, SEEK_SET);
                break;
            }
        }
        if (total_num < 1)
            do_debug(LOG_FATAL, "not enough lines at log file %s.\n", filename);
        memset(&line[0], 0, LEN_10240);
        fgets(line[0], LEN_10240, fp);
    } else {
        /* 得到的是tsar文件里最末尾的两行也就是最新的两行数据 */
        memset(&line[0], 0, LEN_10240);
        fgets(line[0], LEN_10240, fp);
        memset(&line[1], 0, LEN_10240);
        fgets(line[1], LEN_10240, fp);
    }

    /* 初始化mod->record的内存空间 */
    init_module_fields();

    /* 读日志文件倒数第二行到mod->record里 */
    read_line_to_module_record(line[0]);
    collect_record_stat();

    /* 读日志文件倒数第二行到mod->record里 */
    read_line_to_module_record(line[1]);
    collect_record_stat();

    /* display check detail */
    /* --------------------------------RUN_CHECK_NEW----------------------------- */
    if (check_type == RUN_CHECK_NEW) {
        printf("%s\ttsar\t", host_name);
        for (i = 0; i < statis.total_mod_num; i++) {
            mod = &mods[i];
            if (!mod->enable)
                continue;

            struct mod_info *info = mod->info;
            /* get mode name */
            char *mod_name = strstr(mod->opt_line, "--");
            if (mod_name)
                mod_name += 2;

            char opt[LEN_128] = {0};
            char *n_record = strdup(mod->record);
            char *token = strtok(n_record, ITEM_SPLIT);
            char *s_token;

            for (j = 0; j < mod->n_item; j++) {
                memset(opt, 0, sizeof(opt));
                if (token) {
                    /* 有"="的项记录保存至opt */
                    if ((s_token = strstr(token, ITEM_SPSTART))) {
                        strncat(opt, token, s_token - token);
                        strcat(opt, ":");
                    }
                }

                st_array = &mod->st_array[j * mod->n_col];

                for (k = 0; k < mod->n_col; k++) {

                    if (mod->spec) {

                        if (!st_array || !mod->st_flag) {
                            if (((DATA_SUMMARY == conf.print_mode) && (SPEC_BIT == info[k].summary_bit))
                                    || ((DATA_DETAIL == conf.print_mode) && (SPEC_BIT == info[k].summary_bit)))
                                printf("%s:%s%s=-%s", mod_name,opt,trim(info[k].hdr,LEN_128), " ");
                        } else {
                            if (((DATA_SUMMARY == conf.print_mode) && (SPEC_BIT == info[k].summary_bit))
                                    || ((DATA_DETAIL == conf.print_mode) && (SPEC_BIT == info[k].summary_bit))) {
                                printf("%s:%s%s=", mod_name,opt,trim(info[k].hdr,LEN_128));
                                printf("%0.1f ", st_array[k]);
                            }
                        }

                    } else {

                        if (!st_array || !mod->st_flag) {
                            if (((DATA_SUMMARY == conf.print_mode) && (SUMMARY_BIT == info[k].summary_bit))
                                    || ((DATA_DETAIL == conf.print_mode) && (HIDE_BIT != info[k].summary_bit)))
                                printf("%s:%s%s=-%s", mod_name,opt,trim(info[k].hdr,LEN_128), " ");
                        } else {
                            if (((DATA_SUMMARY == conf.print_mode) && (SUMMARY_BIT == info[k].summary_bit))
                                    || ((DATA_DETAIL == conf.print_mode) && (HIDE_BIT != info[k].summary_bit))) {
                                printf("%s:%s%s=", mod_name,opt,trim(info[k].hdr,LEN_128));
                                printf("%0.1f ", st_array[k]);
                            }
                        }

                    }

                }

                if (token)
                    token = strtok(NULL, ITEM_SPLIT);
            }
            if (n_record) {
                free(n_record);
                n_record = NULL;
            }
        }
        printf("\n");
        fclose(fp);
        fp = NULL;
        return;
    }

#ifdef OLDTSAR
    /* --------------------------------RUN_CHECK----------------------------- */
    /* run_check这里有问题,不用太纠结 */
    if (check_type == RUN_CHECK) {
        for (i = 0; i < statis.total_mod_num; i++) {
            mod = &mods[i];
            if (!mod->enable)
                continue;

            if (!strcmp(mod->name, "mod_cpu")) {
                memset(&tmp[1], 0, LEN_256);
                for (j = 0; j < mod->n_col; j++) {
                    st_array = &mod->st_array[j * mod->n_col];
                    if (!st_array || !mod->st_flag)
                        sprintf(tmp[1]," cpu=-");
                    else
                        sprintf(tmp[1]," cpu=%0.2f",st_array[5]);
                }
            }
        }
        for (j = 0; j < 9; j++)
            strcat(check, tmp[j]);
        printf("%s\n", check);
        fclose(fp);
        fp = NULL;
    }
#endif
}
