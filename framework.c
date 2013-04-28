#include "tsar.h"

void register_mod_fileds(struct module *mod, char *opt, char *usage,
		struct mod_info *info, int n_col, void *data_collect, void *set_st_record) {
	sprintf(mod->opt_line, "%s", opt);
	sprintf(mod->usage, "%s", usage);
	mod->info = info;
	mod->n_col = n_col;
	mod->data_collect = data_collect;
	mod->set_st_record = set_st_record;
}


void set_mod_record(struct module *mod, char *record) {
	if (record)
		sprintf(mod->record, "%s", record);
}

void load_modules() {
	char	buff[LEN_128] = {0};
	char	mod_path[LEN_128] = {0};
	struct	module *mod = NULL;
	int	(*mod_register)(struct module *);
	int	i;

	sprintf(buff, conf.module_path);

	for (i = 0; i < statis.total_mod_num; i++) {
		mod = &mods[i];
		if (!mod->lib) {
			snprintf(mod_path, LEN_128, "%s/%s.so", buff, mod->name);
			if (!(mod->lib = dlopen(mod_path, RTLD_NOW|RTLD_GLOBAL))) {
				do_debug(LOG_ERR, "load_modules: dlopen module %s err %s\n", mod->name, dlerror());
			}
			else {
				mod_register = dlsym(mod->lib, "mod_register");
				if (dlerror()) {
					do_debug(LOG_ERR, "load_modules: dlsym module %s err %s\n", mod->name, dlerror());
					break;
				}
				else {
					mod_register(mod);
					mod->enable = 1;
					mod->spec = 0;
					do_debug(LOG_INFO, "load_modules: load new module '%s' to mods\n", mod_path);
				}
			}
		}
	}
}

void collect_record(void) {
    struct module *mod = NULL;
    int i;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->enable)
            continue;
        memset(mod->record, 0, sizeof(mod->record));
        if (mod->data_collect)
            mod->data_collect(mod, mod->parameter);
    }
}

void read_line_to_module_record(char *line) {
    int i;
    struct module *mod;
    char *s_token, *e_token;

    line[strlen(line) - 1] = '\0';
    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (mod->enable) {
            memset(mod->record, 0, sizeof(mod->record));
            s_token = strstr(line, mod->opt_line);
            // 没匹配到opt_line则到for
            if (!s_token)
                continue;

            s_token += strlen(mod->opt_line) + sizeof(STRING_SPLIT) - 1;
            e_token = strstr(s_token, SECTION_SPLIT);

            if (e_token)
                memcpy(mod->record, s_token, e_token - s_token);
            else
                // line的总长 减去 从line的开头到s_token这一段
                memcpy(mod->record, s_token, strlen(line) - (s_token - line));
        }
    }
}

void init_module_fields() {
    int i;
    struct module *mod;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->enable)
            continue;

        if (MERGE_ITEM == conf.print_merge)
            mod->n_item = 1;
        else
            mod->n_item = get_strtok_num(mod->record, ITEM_SPLIT);

        if (mod->n_item) {
            mod->pre_array = (U_64 *)calloc(mod->n_item * mod->n_col, sizeof(U_64));
            mod->cur_array = (U_64 *)calloc(mod->n_item * mod->n_col, sizeof(U_64));
            mod->st_array = (double *)calloc(mod->n_item * mod->n_col, sizeof(double));
            if (conf.print_tail) {
                mod->max_array = (double *)calloc(mod->n_item * mod->n_col, sizeof(double));
                mod->mean_array = (double *)calloc(mod->n_item * mod->n_col, sizeof(double));
                mod->min_array = (double *)calloc(mod->n_item * mod->n_col, sizeof(double));
            }
        }
    }
}

/* 此函数完全没看懂 */
int collect_record_stat(void) {
    return 0;
}

int is_include_string(char *mods, char *mod) {
    char *token, n_str[LEN_512] = {0};

    memcpy(n_str, mods, strlen(mods));
    token = strtok(n_str, DATA_SPLIT);
    while (token) {
        if (!strcmp(token, mod))
            return 1;
        token = strtok(NULL, DATA_SPLIT);
    }
    return 0;
}

int reload_modules(char *s_mod) {
    int reload = 0;
    int i;
    struct module *mod;
    
    if (!s_mod || !strlen(s_mod))
        return reload;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (is_include_string(s_mod, mod->name) || is_include_string(s_mod, mod->opt_line)) {
            mod->enable = 1;
            reload = 1;
        } else
            mod->enable = 0;
    }
    return reload;
}

#ifdef OLDTSAR

void reload_check_modules(void) {
    int i;
    struct module *mod;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!strcmp(mod->name, "apache") || !strcmp(mod->name, "mod_cpu") || !strcmp(mod->name, "mod_mem") || !strcmp(mod->name, "mod_load") || !strcmp(mod->name, "mod_partition") || !strcmp(mod->name, "mod_io") || !strcmp(mod->name, "mod_tcp") || !strcmp(mod->name, "mod_traffic") || !strcmp(mod->name, "mod_nginx"))
            mod->enable = 1;
        else
            mod->enable = 0;
    }
}

#endif
