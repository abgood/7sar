#ifndef TSAR_DEFINE_H
#define TSAR_DEFINE_H

#define DEFAULT_CONF_FILE_PATH "tsar.conf"

#define MAX_MOD_NUM 32
#define MAX_COL_NUM 64

#define LEN_32 32
#define LEN_64 64
#define LEN_128 128
#define LEN_256 256
#define LEN_512 512
#define LEN_1024 1024
#define LEN_4096 4096
#define LEN_10240 10240

#define TRUE 1
#define FALSE 0

#define W_SPACE " \t\r\n"

enum {
    HIDE_BIT,
    DETAIL_BIT,
    SUMMARY_BIT,
    SPEC_BIT
};

#endif
