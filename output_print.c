#include "tsar.h"

void running_check(int check_type) {
    char host_name[LEN_64] = {0};
    char check[LEN_10240] = {0};
    char filename[LEN_128] = {0};
    char line[2][10240];
    int i, j, total_num = 0;
    FILE *fp;
    struct module *mod;
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

    /* 收集信息这块有点小问题 */
    /* set struct module fields*/
    read_line_to_module_record(line[0]);
    init_module_fields();

    collect_record_stat();

    read_line_to_module_record(line[1]);
    collect_record_stat();

    /* -----------------------------------RUN_CHECK_NEW------------------------------ */
    if (check_type == RUN_CHECK_NEW) {
        printf("%s\ttsar\t", host_name);
    }

#ifdef OLDTSAR
    /* -----------------------------------RUN_CHECK------------------------------ */
    if (check_type == RUN_CHECK) {

        for (i = 0; i < statis.total_mod_num; i++) {
            mod = &mods[i];
            if (!mod->enable)
                continue;

            if (!strcmp(mod->name, "mod_apache")) {
                ;
            }

            if (!strcmp(mod->name, "mod_cpu")) {
                for (j = 0; j < mod->n_col; j++) {
                    st_array = &mod->st_array[j * mod->n_col];
                    if (!st_array || !mod->st_flag)
                        sprintf(tmp[0], " apache/qps=- apache/rt=- apache/busy=- apache/idle=-");
                    else 
                        sprintf(tmp[0], " apache/qps=%0.2f apache/rt=%0.2f apache/busy=%0.0f apache/idle=%0.0f", st_array[0], st_array[1], st_array[2], st_array[3]);
                }
            }

            if (!strcmp(mod->name, "mod_mem")) {
                ;
            }

            if (!strcmp(mod->name, "mod_load")) {
                ;
            }

            if (!strcmp(mod->name, "mod_io")) {
                ;
            }

            if (!strcmp(mod->name, "mod_traffic")) {
                ;
            }

            if (!strcmp(mod->name, "mod_tcp")) {
                ;
            }

            if (!strcmp(mod->name, "mod_partition")) {
                ;
            }

            if (!strcmp(mod->name, "mod_nginx")) {
                ;
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
