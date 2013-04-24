#ifndef TSAR_FRAMEWORK_H
#define TSAR_FRAMEWORK_H

#include "define.h"

struct mod_info {
    char hdr[LEN_128];
    int summary_bit;
    int merge_mode;
    int stats_opt;
};

struct module {
    char name[LEN_32];
    char parameter[LEN_256];
    struct mod_info *info;
    int n_col;
    int spec;
    char usage[LEN_256];
};

void load_modules(void);

#endif