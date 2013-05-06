#ifndef TSAR_DEFINE_H
#define TSAR_DEFINE_H

#define OLDTSAR

#define U_64 unsigned long long

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
#define DATA_SPLIT ","
#define SECTION_SPLIT "|"
#define STRING_SPLIT ":"
#define ITEM_SPLIT ";"
#define ITEM_SPSTART "="
#define DATA_SPLIT ","
#define PRINT_SEC_SPLIT " "
#define PRINT_DATA_SPLIT "  "

#define DEFAULT_PRINT_INTERVAL 5
#define DEFAULT_PRINT_NUM 20

#define VMSTAT "/proc/vmstat"
#define STAT "/proc/stat"

enum {
    HIDE_BIT,     /* 0 */
    DETAIL_BIT,   /* 1 */
    SUMMARY_BIT,  /* 2 */
    SPEC_BIT      /* 3 */
};

enum {
    RUN_NULL,        /* 0 */
    RUN_LIST,        /* 1 */
    RUN_CRON,        /* 2 */
#ifdef OLDTSAR
    RUN_CHECK,       /* 3 */
    RUN_CHECK_NEW,   /* 4 */
#endif
    RUN_PRINT,       /* 5 */
    RUN_PRINT_LIVE   /* 6 */
};

enum {
    DATA_NULL,
    DATA_SUMMARY,
    DATA_DETAIL,
    DATA_ALL
};

enum {
    MERGE_NOT,
    MERGE_ITEM
};

enum {
    STATS_NULL,
    STATS_SUB,
    STATS_SUB_INTER
};

enum {
    MERGE_NULL,
    MERGE_SUM,
    MERGE_AVG
};

#endif
