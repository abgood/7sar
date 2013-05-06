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


/* find where start printting
 * return
 * 0 ok
 * 1 need find last tsar.data file
 * 2 find error, find time is later than the last line at tsar.data.x, should stop find any more
 * 3 find error, tsar haved stopped after find time, should stop find it
 * 4 find error, data not exist, tsar just lost some time data which contains find time
 * 5 log format error
 * 6 other error
 */
int find_offset_from_start(FILE *fp, int number) {
    long fset, fend, file_len, off_start, off_end, offset, line_len;
    char line[LEN_10240] = {0};
    time_t now, t_token, t_get;
    struct tm stm;
    char *p_sec_token;

    /* get file len */
    fseek(fp, 0, SEEK_END);
    fend = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fset = ftell(fp);
    /* 文件的总长度 */
    file_len = fend - fset;

    memset(&line, 0, LEN_10240);
    fgets(line, LEN_10240, fp);
    /* 文件一行的长度 */
    line_len = strlen(line);

    /* get time token */
    time(&now);
	if(conf.print_day > 180){
		/*get specify date by --date/-d*/
		stm.tm_year = conf.print_day / 10000 - 1900;
		stm.tm_mon = conf.print_day % 10000 / 100 - 1;
		stm.tm_mday = conf.print_day % 100;
		t_token = mktime(&stm);
		conf.print_day = (now - t_token) / (24 * 60 * 60);
	}
	if(conf.print_day >= 0){
        /* 求出-d设置的几天前的那一天的时间范围 */
		if(conf.print_day > 180)
			conf.print_day = 180;
		/* get day's beginning plus 8 hours.Set start and end time for print */
		now = now - now % (24 * 60 * 60) - (8 * 60 * 60);
		t_token = now - conf.print_day * (24 * 60 * 60) - (60 * conf.print_nline_interval);
		conf.print_start_time = t_token;
		conf.print_end_time = t_token + 24 * 60 * 60 + (60 * conf.print_nline_interval);
	}else{
		/* set max days for print 6 months */
		if(conf.print_ndays > 180)
			conf.print_ndays = 180;
		now = now - now % (60 * conf.print_nline_interval);
		t_token = now - conf.print_ndays * (24 * 60 * 60) - (60 * conf.print_nline_interval);
		conf.print_start_time = t_token;
		conf.print_end_time = now + (60 * conf.print_nline_interval);
	}

    offset = off_start = 0;
    off_end = file_len;
    /* 1.定位的tsar.data文件中部
     * 2.找出文件中部下一行的时间戳
     * 3.计算中部时间戳是否与查找几天前的开始时间相差60秒以内
     * 4.中部时间戳与查找开始时间比较
     *  4.1 大于往上查找
     *  4.2 小于往下查找
     */
    while (1) {
        offset = (off_start + off_end) / 2;
        memset(&line, 0, LEN_10240);
        /* 定位到文件中部 */
        fseek(fp, offset, SEEK_SET);
        fgets(line, LEN_10240, fp);
        memset(&line, 0, LEN_10240);
        fgets(line, LEN_10240, fp);
        if (0 != line[0] && offset > line_len) {
            if ((p_sec_token = strstr(line, SECTION_SPLIT))) {
                *p_sec_token = '\0';
                t_get = atol(line);
                if (abs(t_get - t_token) <= 60) {
                    conf.print_file_number = number;
                    return 0;
                }
                if (t_get > t_token)
                    off_end = offset;
                else if (t_get < t_token)
                    off_start = offset;
            } else
                return 5;
        } else {
            /* 往下查找到文件尾了,说明此文件很比查找时间还要老,换文件查找tsar.data.(number - 1) */
            if (off_end == file_len) {
                if (number > 0) {
                    conf.print_file_number = number - 1;
                    return 2;
                } else
                    return 3;
            }
            /* 往上查找到文件头 */
            if (off_start == 0) {
                conf.print_file_number = number;
                return 1;
            }
            return 6;
        }

        if (offset == (off_start + off_end) / 2) {
            if (off_start != 0) {
                conf.print_file_number = number;
                return 4;
            }
            return 6;
        }
    }
}

void adjust_print_opt_line(char *n_opt_line, char *opt_line, int hdr_len) {
    char pad[LEN_128] = {0};
    int pad_len;

    if (hdr_len > strlen(opt_line)) {
        pad_len = (hdr_len - strlen(opt_line)) / 2;
        memset(pad, '-', pad_len);
        strcat(n_opt_line, pad);
        strcat(n_opt_line, opt_line);
        memset(&pad, '-', hdr_len - pad_len - strlen(opt_line));
        strcat(n_opt_line, pad);
    } else
        strncat(n_opt_line, opt_line, hdr_len);
}

/* print header and update mod->n_item */
void print_header(void) {
    struct module *mod = NULL;
    char opt_line[LEN_10240] = {0};
    char hdr_line[LEN_10240] = {0};
    char opt[LEN_128] = {0};
    char n_opt[LEN_256] = {0};
    char mod_hdr[LEN_256] = {0};
    char *token, *s_token, *n_record;
    char header[LEN_10240] = {0};
    int i;

    if (conf.running_mode == RUN_PRINT_LIVE) {
        sprintf(opt_line, "Time             %s", PRINT_SEC_SPLIT);
        sprintf(hdr_line, "Time             %s", PRINT_SEC_SPLIT);
    } else {
        sprintf(opt_line, "Time          %s", PRINT_SEC_SPLIT);
        sprintf(hdr_line, "Time          %s", PRINT_SEC_SPLIT);
    }

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->enable)
            continue;

        get_mod_hdr(mod_hdr, mod);

        if (strstr(mod->record, ITEM_SPLIT) && MERGE_NOT == conf.print_merge) {
            n_record = strdup(mod->record);
            token = strtok(n_record, ITEM_SPLIT);
            while (token) {
                s_token = strstr(token, ITEM_SPSTART);
                if (s_token) {
                    memset(opt, 0, sizeof(opt));
                    memset(n_opt, 0, sizeof(n_opt));
                    strncat(opt, token, s_token - token);
                    adjust_print_opt_line(n_opt, opt, strlen(mod_hdr));
                    strcat(opt_line, n_opt);
                    strcat(opt_line, PRINT_SEC_SPLIT);
                    strcat(hdr_line, mod_hdr);
                    strcat(hdr_line, PRINT_SEC_SPLIT);
                }
                token = strtok(NULL, ITEM_SPLIT);
            }
        } else {
            memset(opt, 0, sizeof(opt));
            adjust_print_opt_line(opt, mod->opt_line, strlen(mod_hdr));

            strcat(hdr_line, mod_hdr);
            strcat(opt_line, opt);
        }
        strcat(hdr_line, PRINT_SEC_SPLIT);
        strcat(opt_line, PRINT_SEC_SPLIT);
    }
    sprintf(header, "%s\n%s\n", opt_line, hdr_line);
    printf("%s", header);
}

/* set record time */
long set_record_time(char *line) {
    char *token, s_time[LEN_32] = {0};
    static long pre_time, c_time = 0;

    /* get record time */
    token = strtok(line, SECTION_SPLIT);
    memcpy(s_time, line, token - line);

    /* swap time */
    pre_time = c_time;
    c_time = atol(s_time);

    c_time = c_time - c_time % 60;
    pre_time = pre_time - pre_time % 60;

    if (!(conf.print_interval = c_time - pre_time))
        return 0;
    else
        return c_time;
}

FILE *init_running_print() {
    FILE *fp, *fptmp;
    int i = 0, k = 0;
    char filename[LEN_128] = {0};
    char line[LEN_10240] = {0};

    conf.print_tail = 1;

    if (!(fp = fopen(conf.output_file_path, "r")))
        do_debug(LOG_FATAL, "unable to open the log file %s\n", conf.output_file_path);

    conf.print_file_number = -1;

    /* find start offset will print from tsar.data */
    k = find_offset_from_start(fp, i);
    /* 往上查找没找到,说明查找的日期再老文件里 */
    if (k == 1) {
        for (i = 1;;i++) {
            /* i越大,tsar.data.i越老 */
            sprintf(filename, "%s.%d", conf.output_file_path, i);
            if(!(fptmp = fopen(filename, "r"))) {
                conf.print_file_number = i - 1;
                break;
            }

            k = find_offset_from_start(fp, i);
            if (k == 0 || k == 4) {
                fclose(fp);
                fp = fptmp;
                break;
            }

            if (k == 2) {
                fseek(fp, 0, SEEK_SET);
                fclose(fptmp);
                break;
            }

            if (k == 1) {
                fclose(fp);
                fp = fptmp;
                break;
            }

            if (k == 5 || k == 6)
                do_debug(LOG_FATAL, "log format error or find_offset_from_start have a bug. error code=%d\n", k);
        }
    }
    if (k == 5 || k == 6)
        do_debug(LOG_FATAL, "log format error or find_offset_from_start have a bug. error code=%d\n", k);

    /* get records */
    if (!fgets(line, LEN_10240, fp))
        do_debug(LOG_FATAL, "can't get enough log info\n");

    read_line_to_module_record(line);

    /* print header */
    print_header();

    init_module_fields();

    set_record_time(line);
    return fp;
}


/* print mode, print data from tsar.data */
void running_print() {
    FILE *fp;
    char filename[LEN_128] = {0};
    char line[LEN_10240] = {0};

    fp = init_running_print();

    if (collect_record_stat() == 0)
        do_debug(LOG_INFO, "collect_record_stat warn\n");

    while (1) {
        if (!fgets(line, LEN_10240, fp)) {
            if (conf.print_file_number <= 0)
                break;
            else {
                conf.print_file_number = conf.print_file_number - 1;
                memset(filename, 0, sizeof(filename));
                if (conf.print_file_number == 0)
                    sprintf(filename, "%s", conf.output_file_path);
                else
                    sprintf(filename, "%s%d", conf.output_file_path, conf.print_file_number);
                fclose(fp);
                if(!(fp = fopen(filename, "r")))
                    do_debug(LOG_FATAL, "unable to open the log file %s.\n", filename);
                continue;
            }
        }

        // int k = check_time(line);
    }
}
