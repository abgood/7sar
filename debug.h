#ifndef TSAR_DEBUG_H
#define TSAR_DEBUG_H

typedef enum {
    LOG_INFO,   /* 0 */
    LOG_DEBUG,  /* 1 */
    LOG_WARN,   /* 2 */
    LOG_ERR,    /* 3 */
    LOG_FATAL   /* 4 */
} log_level_t;

void do_debug(log_level_t, const char *, ...);

#endif
