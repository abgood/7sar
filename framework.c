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

/* 重分配存储,当mod->n_item是被改变 */
void realloc_module_array(struct module *mod, int n_n_item) {
    if (n_n_item > mod->n_item) {
        if (mod->pre_array) {
            mod->pre_array = (U_64 *)realloc(mod->pre_array, n_n_item * mod->n_col * sizeof(U_64));
            mod->cur_array = (U_64 *)realloc(mod->cur_array, n_n_item * mod->n_col * sizeof(U_64));
            mod->st_array = (double *)realloc(mod->st_array, n_n_item * mod->n_col * sizeof(double));
            if (conf.print_tail) {
                mod->max_array = (double *)realloc(mod->max_array, n_n_item * mod->n_col * sizeof(double));
                mod->mean_array = (double *)realloc(mod->mean_array, n_n_item * mod->n_col * sizeof(double));
                mod->min_array = (double *)realloc(mod->min_array, n_n_item * mod->n_col * sizeof(double));
            }
        } else {
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

/* set st result in st_array */
void set_st_record(struct module *mod) {
    int i, j, k = 0;
    struct mod_info *info = mod->info;
    mod->st_flag = 1;

    for (i = 0; i < mod->n_item; i++) {
        if (mod->set_st_record) {
            mod->set_st_record(mod, &mod->st_array[i * mod->n_col],
                    &mod->pre_array[i * mod->n_col],
                    &mod->cur_array[i * mod->n_col],
                    conf.print_interval);
        }

        for (j = 0; j < mod->n_col; j++) {
            if (!mod->set_st_record) {
                switch (info[j].stats_opt) {
                    case STATS_SUB:
                        if (mod->cur_array[k] < mod->pre_array[k]) {
                            mod->pre_array[k] = mod->cur_array[k];
                            mod->st_flag = 0;
                        } else
                            mod->st_array[k] = mod->cur_array[k] - mod->pre_array[k];
                        break;
                    case STATS_SUB_INTER:
                        if (mod->cur_array[k] < mod->pre_array[k]) {
                            mod->pre_array[k] = mod->cur_array[k];
                            mod->st_flag = 0;
                        } else
                            mod->st_array[k] = (mod->cur_array[k] - mod->pre_array[k]) / conf.print_interval;
                        break;
                    default:
                        mod->st_array[k] = mod->cur_array[k];
                }
            mod->st_array[k] *= 1.0;
            }

            if (conf.print_tail) {
                if (0 == mod->record)
                    mod->max_array[k] = mod->mean_array[k] = mod->min_array[k] = mod->st_array[k] * 1.0;
                else {
                    if (mod->st_array[k] - mod->max_array[k] > 0.1)
                        mod->max_array[k] = mod->st_array[k];
                    if (mod->min_array[k] - mod->st_array[k] > 0.1 && mod->st_array[k] >= 0)
                        mod->min_array[k] = mod->st_array[k];
                    if(mod->st_array[k] >= 0)
                        mod->mean_array[k] = ((mod->n_record-1) *mod->mean_array[k] + mod->st_array[k])/mod->n_record;
                }
            }
            k++;
        }
    }
    mod->n_record++;
}

/* 此函数完全没看懂 */
int collect_record_stat(void) {
    struct module *mod = NULL;
    int i, n_item, ret, no_p_hdr = 1;
    U_64 *tmp, array[MAX_MOD_NUM];

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->enable)
            continue;

        memset(array, 0, sizeof(array));
        mod->st_flag = 0;
        ret = 0;

        /* 一个模块里可能监测多个项,每个项是通过分号来分割的 */
        if ((n_item = get_strtok_num(mod->record, ITEM_SPLIT))) {
            /* not merge mode, and last n_item != cur n_item, then reset mod->n_item and set reprint header flag */
            if (MERGE_ITEM != conf.print_merge && n_item && n_item != mod->n_item) {
                no_p_hdr = 0;
                realloc_module_array(mod, n_item);
            }

            mod->n_item = n_item;
            /* 多个项是用分号分开的 */
            if (strstr(mod->record, ITEM_SPLIT)) {
                /* 合并项 */
                if (MERGE_ITEM == conf.print_merge) {
                    mod->n_item = 1;
                    ret = merge_mult_item_to_array(mod->cur_array, mod);
                } else {
                    char item[LEN_128] = {0};
                    int num = 0;
                    int pos = 0;

                    while (strtok_next_item(item, mod->record, &pos)) {
                        if (!(ret = convert_record_to_array(&mod->cur_array[num * mod->n_col], mod->n_col, item)))
                            break;
                        memset(item, 0, sizeof(item));
                        num++;
                    }
                }
            } else {    /* ont item */
                ret = convert_record_to_array(mod->cur_array, mod->n_col, mod->record);
            }

            /* get st record */
            if (no_p_hdr && mod->pre_flag && ret)
                set_st_record(mod);

            if (!ret)
                mod->pre_flag = 0;
            else
                mod->pre_flag = 1;
        } else 
            mod->pre_flag = 0;

        /* swap cur_array to pre_curry */
        tmp = mod->pre_array;
        mod->pre_array = mod->cur_array;
        mod->cur_array = tmp;
    }
    return no_p_hdr;
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
