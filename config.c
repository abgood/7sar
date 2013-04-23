#include "tsar.h"

void parse_mod(char *mod_name) {
    int i;

    for (i = 0; i < statis.total_mod_num; i++) {
        struct module *mod = &mods[i];
        if (!strcmp(mod->name, mod_name))
            return;
    }
    struct module *mod = &mods[statis.total_mod_num++];
    char *token = strtok(NULL, W_SPACE);
    if (token && (!strcasecmp(token, "on") || !strcasecmp(token, "enable"))) {
        strncpy(mod->name, mod_name, strlen(mod_name));
        token = strtok(NULL, W_SPACE);
        if (token)
            strncpy(mod->parameter, token, strlen(token));
        return;
    } else {
        memset(mod, 0, sizeof(struct module));
        statis.total_mod_num--;
    }
}

/* 这个函数看的不是太明白 */
void special_mod(char *spec_mod) {
    int i, j;
    char mod_name[LEN_32];
    struct module *mod = NULL;
    memset(mod_name, 0, LEN_32);
    sprintf(mod_name, "mod_%s", spec_mod + 5);
    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!strcmp(mod->name, mod_name)) {
            //load_modules();
            char *token = strtok(NULL, W_SPACE);
            struct mod_info *info = mod->info;
            for (j = 0; j < mod->n_col; j++) {
                char *p = info[j].hdr;
                while (*p == ' ') p++;
                if (strstr(token, p)) {
                    info[j].summary_bit = SPEC_BIT;
                    mod->spec = 1;
                }
            }
        }
    }
}

void parse_string(char *var) {
    char *token = strtok(NULL, W_SPACE);

    if (token)
        strncpy(var, token, strlen(token));
}

/* 与原来代码不同 */
void parse_add_string(char *var) {
    char *token = strtok(NULL, W_SPACE);
    
    if (token) {
        strcat(token, ",");
        strncat(token, var, strlen(var));
    }
    if (token)
        strncpy(var, token, strlen(token));
}

static int parse_line(char *buff) {
    char *token;

    if ((token = strtok(buff, W_SPACE)) == NULL)
        (void) 0;
    else if (strstr(token, "mod_"))
        parse_mod(token);
    else if (strstr(token, "spec_"))
        special_mod(token);
    else if (!strcmp(token, "output_interface"))
        parse_string(conf.output_interface);
    else if (!strcmp(token, "output_file_path"))
        parse_string(conf.output_file_path);
    else if (!strcmp(token, "output_db_addr"))
        parse_string(conf.output_db_addr);
    else if (!strcmp(token, "output_db_mod"))
        parse_add_string(conf.output_db_mod);
    else
        return 0;
    return 1;
}

void parse_config_file(const char *file_name) {
    FILE *fp;
    char config_input_line[LEN_1024];
    char *token;

    if (!(fp = fopen(file_name, "r")))
        do_debug(LOG_FATAL, "Unable to open configuration file: %s\n", file_name);

    memset(&conf, '\0', sizeof(conf));
    memset(&mods, '\0', sizeof(mods));
    memset(&statis, '\0', sizeof(statis));
    conf.server_port = (int *)malloc(sizeof(int));
    conf.cycle_time = (int *)malloc(sizeof(int));
    //程序在这里设置debug_level为3,而到后面进行判断tsar.conf关键字时不能打印出错信息
    //conf.debug_level = LOG_ERR;
    conf.debug_level = LOG_INFO;
    conf.print_detail = FALSE;
    while (fgets(config_input_line,LEN_1024, fp)) {
        if ((token = strchr(config_input_line, '\n')))
            *token = '\0';
        if ((token = strchr(config_input_line, '\r')))
            *token = '\0';
        if (config_input_line[0] == '#') {
            memset(config_input_line,'\0', LEN_1024);
            continue;
        }
        if (config_input_line[0] == '\0')
            continue;
        if (!parse_line(config_input_line))
            do_debug(LOG_INFO, "parse_config_file: unknown keyword in '%s' \n", config_input_line);
        memset(config_input_line, '\0', LEN_1024);
    }
    fclose(fp);
}
