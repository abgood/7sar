#include "tsar.h"

void running_check(int check_type) {
    char host_name[LEN_64] = {0};
    char check[LEN_10240] = {0};
    char filename[LEN_128] = {0};
    char line[2][10240];
    int i, total_num = 0;
    FILE *fp;

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
}
