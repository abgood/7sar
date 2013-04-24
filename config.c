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

void set_debug_level() {
    char *token = strtok(NULL, W_SPACE);

    if (token) {
        if (!strcmp(token, "INFO"))
            conf.debug_level = LOG_INFO;
        else if (!strcmp(token, "WARN"))
            conf.debug_level = LOG_WARN;
        else if (!strcmp(token, "DEBUG"))
            conf.debug_level = LOG_DEBUG;
        else if (!strcmp(token, "ERROR"))
            conf.debug_level = LOG_ERR;
        else if (!strcmp(token, "FATAL"))
            conf.debug_level = LOG_FATAL;
        else
            conf.debug_level = LOG_ERR;
    }
}

void parse_int(int *var) {
    char *token = strtok(NULL, W_SPACE);

    if (token == NULL)
        do_debug(LOG_FATAL, "Bungled line");
    *var = strtol(token, NULL, 0);
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
    else if (!strcmp(token, "output_nagios_mod"))
        parse_add_string(conf.output_nagios_mod);
    else if (!strcmp(token, "output_stdio_mod"))
        parse_add_string(conf.output_stdio_mod);
    else if (!strcmp(token, "debug_level"))
        set_debug_level();
    else if (!strcmp(token, "include"))
        get_include_conf();
    else if (!strcmp(token, "server_addr"))
        parse_string(conf.server_addr);
    else if (!strcmp(token, "server_port"))
        parse_int(conf.server_port);
    else if (!strcmp(token, "cycle_time"))
        parse_int(conf.cycle_time);
    else if (!strcmp(token, "send_nsca_cmd"))
        parse_string(conf.send_nsca_cmd);
    else if (!strcmp(token, "send_nsca_conf"))
        parse_string(conf.send_nsca_conf);
    else if (!strcmp(token, "threshold"))
        get_threshold();
    else
        return 0;
    return 1;
}

void get_include_conf() {
    char *token = strtok(NULL, W_SPACE);
    char cmd[LEN_1024] = {0};
    char buf[LEN_1024] = {0};
    FILE *stream, *fp;
    char *tmp, *p;
    char config_input_line[LEN_1024] = {0};

    if (token) {
        sprintf(cmd, "ls %s 2>/dev/null", token);
        if (strchr(cmd, ';') != NULL || strchr(cmd, '|') != NULL || strchr(cmd, '&') != NULL)
            do_debug(LOG_ERR, "include formart Error:%s\n", cmd);
        stream = popen(cmd, "r");
        if (stream == NULL) {
            do_debug(LOG_ERR, "popen failed. Error:%s\n", strerror(errno));
            return;
        }
        while (fgets(buf, LEN_1024, stream)) {
            do_debug(LOG_INFO, "parse file %s", buf);
            p = buf;
            while (p) {
                if (*p == '\r' || *p == '\n') {
                    *p = '\0';
                    break;
                }
                p++;
            }
            if (!(fp = fopen(buf, "r"))) {
                do_debug(LOG_ERR, "Unable to open configuration file: %s Error msg: %s\n", buf, strerror(errno));
                continue;
            }
            while (fgets(config_input_line, LEN_1024, fp)) {
                if ((tmp = strchr(config_input_line, '\n')))
                    *tmp = '\0';
                if ((tmp = strchr(config_input_line, '\r')))
                    *tmp = '\0';
                if (config_input_line[0] == '#') {
                    memset(config_input_line, '\0', LEN_1024);
                    continue;
                }
                if (config_input_line[0] == '\0')
                    continue;
                if (!parse_line(config_input_line))
                    do_debug(LOG_INFO, "parse_config_file: unknown keyword in '%s' at file %s\n", config_input_line, buf);
                memset(config_input_line, '\0', LEN_1024);
            }
            fclose(fp);
        }
        if (pclose(stream) == -1)
            do_debug(LOG_WARN, "pclose error\n");
    }
}

void get_threshold() {
    char *token = strtok(NULL, W_SPACE);
    char tmp[4][LEN_32];

    if (conf.mod_num >= MAX_MOD_NUM)
        do_debug(LOG_FATAL, "Too many mod threshold\n");
    sscanf(token, "%[^;];%[.N0-9];%[.N0-9];%[.N0-9];%[.N0-9];", conf.check_name[conf.mod_num], tmp[0], tmp[1], tmp[2], tmp[3]);
    if (!strcmp(tmp[0], "N"))
        conf.wmin[conf.mod_num] = 0;
    else 
        conf.wmin[conf.mod_num] = atof(tmp[0]);
    if (!strcmp(tmp[1], "N"))
        conf.wmax[conf.mod_num] = 0;
    else 
        conf.wmax[conf.mod_num] = atof(tmp[1]);
    if (!strcmp(tmp[2], "N"))
        conf.cmin[conf.mod_num] = 0;
    else 
        conf.cmin[conf.mod_num] = atof(tmp[2]);
    if (!strcmp(tmp[3], "N"))
        conf.cmax[conf.mod_num] = 0;
    else 
        conf.cmax[conf.mod_num] = atof(tmp[3]);
    conf.mod_num++;
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
    //这里更正debug_level等级是在tsar.conf文件里设置
    conf.debug_level = LOG_ERR;
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

void set_special_field(char *s) {
    int i, j;
    struct module *mod = NULL;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        struct mod_info *info = mod->info;
        for (j = 0; j < mod->n_col; j++) {
            char *p = info[j].hdr;
            while (*p == ' ') p++;
            if (strstr(s, p)) {
                info[j].summary_bit = SPEC_BIT;
                mod->spec = 1;
            }
        }
    }
}
