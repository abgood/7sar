#ifndef TSAR_H
#define TSAR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>

#include "define.h"
#include "config.h"
#include "debug.h"
#include "framework.h"

struct statistic {
    int total_mod_num;
    time_t cur_time;
};

extern struct configure conf;
extern struct statistic statis;
extern struct module mods[MAX_MOD_NUM];

#endif
