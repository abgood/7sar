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
};

void parse_config_file(const char *);

#endif
