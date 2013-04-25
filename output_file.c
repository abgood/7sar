#include "tsar.h"

void output_file(void) {
    struct module *mod;
    FILE *fp = NULL;
    char line[LEN_10240] = {0};
    char detail[LEN_4096] = {0};
    char s_time[LEN_256] = {0};
    int i, ret = 0;

    if (!(fp = fopen(conf.output_file_path, "a+"))) {
        if (!(fp = fopen(conf.output_file_path, "w")))
            do_debug(LOG_FATAL, "output_file: can't create data file = %s  err=%d\n", conf.output_file_path, errno);
    }

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
    
    if (ret) {
        if (fputs(line, fp) < 0)
            do_debug(LOG_WARN, "write line error\n");
    }
    fclose(fp);
    if (chmod(conf.output_file_path, 0666) < 0)
        do_debug(LOG_WARN, "chmod file %s error\n", conf.output_file_path);
}
