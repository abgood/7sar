#ifndef TSAR_CONFIG_H
#define TSAR_CONFIG_H

#include "define.h"

struct configure {
    int debug_level;
    int *server_port;
    int *cycle_time;
    int print_detail;
};

void parse_config_file(const char *);

#endif
