#ifndef TSAR_CONFIG_H
#define TSAR_CONFIG_H

#include "define.h"

struct configure {
    int debug_level;
    int *server_port;
    int *cycle_time;
    int print_detail;

    char output_interface[LEN_128];
    char output_file_path[LEN_128];
    char output_db_addr[LEN_512];
    char output_db_mod[LEN_512];
    char output_nagios_mod[LEN_512];
    char output_stdio_mod[LEN_512];
    char output_print_mod[LEN_512];
    char config_file[LEN_512];

    char server_addr[LEN_512];
    char send_nsca_cmd[LEN_512];
    char send_nsca_conf[LEN_512];

    int mod_num;
    char check_name[MAX_MOD_NUM][LEN_32];

    float wmin[MAX_MOD_NUM];
    float wmax[MAX_MOD_NUM];
    float cmin[MAX_MOD_NUM];
    float cmax[MAX_MOD_NUM];

    int running_mode;
    int print_mode;
    int print_interval;
    int print_nline_interval;
    int print_day;
    int print_ndays;
    int print_merge;
    int print_start_time;
    int print_end_time;
    int print_tail;
    int print_file_number;
};

void parse_config_file(const char *);
void get_include_conf(void);
void get_threshold(void);
void set_special_field(char *);

#endif
