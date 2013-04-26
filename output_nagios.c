#include "tsar.h"

void output_nagios(void) {
    int now_time;
    char host_name[LEN_64] = {0};
    int i = 0;

    now_time = statis.cur_time - statis.cur_time%60;
    /* 轮转时间是否正常 */
    if ((*conf.cycle_time) == 0 || now_time%*(conf.cycle_time) != 0)
        return;

    if (0 != gethostname(host_name, sizeof(host_name)))
        do_debug(LOG_FATAL, "send to nagios: gethostname err, errno=%d \n", errno);
    /* 验证host_name的每个字符是不是可打印字符 */
    while(host_name[i]) {
        if (!isprint(host_name[i++])) {
            host_name[i - 1] = '\0';
            break;
        }
    }

    conf.print_merge = MERGE_NOT;
    if (get_st_array_from_file(0))
        return;
}
