#include "tsar.h"

/* 返回数据库连接状态 */
MYSQL *conn_mysql(char *db_addr) {
    const char *ip;
    int port;
    MYSQL mysql, *conn;

    // 取得ip和端口
    ip = strtok(db_addr, ":");
    port = atoi(strtok(NULL, ":"));

    // 初始mysql结构
    mysql_init(&mysql);

    // 连接mysql
    if ((conn = mysql_real_connect(&mysql, ip, "root", "123456", "tsar", port, NULL, 0)) == NULL) {
        if (mysql_error(&mysql))
            fprintf(stderr, "connection error %d : %s\n", mysql_errno(&mysql), mysql_error(&mysql));
        do_debug(LOG_ERR, "fail to connect mysql, ip:%s\tport:%d\n", ip, port);
        return NULL;
    }

    return conn;
}
