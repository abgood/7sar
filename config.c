#include "tsar.h"

void parse_config_file(const char *file_name) {
    FILE *fp;

    if (!(fp = fopen(file_name, "r")))
        do_debug();
}
