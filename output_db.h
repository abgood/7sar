#ifndef TSAR_OUTPUT_DB_H
#define TSAR_OUTPUT_DB_H

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <mysql/mysql.h>

void output_db(void);

#endif
