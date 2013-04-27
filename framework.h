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
    char usage[LEN_256];
    char record[LEN_4096];
    char opt_line[LEN_32];

    struct mod_info *info;

    int n_col;
    int n_item;
    long n_record;

    int spec;
    int enable;
    void *lib;

    U_64 *pre_array;
    U_64 *cur_array;
    double *st_array;
    double *max_array;
    double *mean_array;
    double *min_array;

    int pre_flag:4;
    int st_flag:4;

    void (*mod_register) (struct module *);
    void (*data_collect) (struct module *, char *);
    void (*set_st_record) (struct module *, double *, U_64 *, U_64 *, int);
};

void load_modules(void);
void collect_record(void);
void register_mod_fileds(struct module *, char *, char *, struct mod_info *, int, void *, void *);
void set_mod_record(struct module *, char *);
void read_line_to_module_record(char *);
void init_module_fields(void);
int collect_record_stat(void);
int reload_modules(char *);

#endif
