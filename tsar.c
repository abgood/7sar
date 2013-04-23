#include "tsar.h"

struct configure conf;
struct module mods[MAX_MOD_NUM];
struct statistic statis;

int main (int argc, char **argv) {

    parse_config_file(DEFAULT_CONF_FILE_PATH);

    return 0;
}
