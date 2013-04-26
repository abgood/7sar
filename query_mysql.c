#include "tsar.h"

/* 返回数据库连接状态 */
int query_mysql(char *db_addr, const char *sql) {
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
        return 1;
    }

    /* 插入数据 */
    if (mysql_query(conn, sql) != 0) {
        if (mysql_error(conn))
            fprintf(stderr, "connection error %d : %s\n", mysql_errno(conn), mysql_error(conn));
        do_debug(LOG_FATAL, "fail to insert data!\n");
    }
    else
        printf("insert data sucess!\n");

    return 0;
}
