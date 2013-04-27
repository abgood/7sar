#ifndef TSAR_H
#define TSAR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <getopt.h>
#include <dlfcn.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <mysql/mysql.h>

#include "define.h"
#include "config.h"
#include "debug.h"
#include "framework.h"
#include "output_file.h"
#include "output_db.h"
#include "output_nagios.h"
#include "query_mysql.h"
#include "common.h"

struct statistic {
    int total_mod_num;
    time_t cur_time;
};

extern struct configure conf;
extern struct statistic statis;
extern struct module mods[MAX_MOD_NUM];

#endif
