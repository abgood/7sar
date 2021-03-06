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

/*
 * module name must be composed by alpha/number/_
 * match return 1
 */
int is_include_string(char *mods, char *mod)
{
	char *token, n_str[LEN_512] = {0};

	memcpy(n_str, mods, strlen(mods));

	token = strtok(n_str, DATA_SPLIT);
	while (token) {
		if (!strcmp(token, mod)) {
			return 1;
		}
		token = strtok(NULL, DATA_SPLIT);
	}
	return 0;
}

/*
 * reload modules by mods, if not find in mods, then set module disable 
 * return 1 if mod load ok
 * return 0 else
 */
int reload_modules(char *s_mod)
{
	int	i;
	int reload = 0;
	struct	module *mod;

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
/*
 * reload check modules by mods, if not find in mods, then set module disable 
 */
void reload_check_modules()
{
	int     i;  
	struct  module *mod;

	for (i = 0; i < statis.total_mod_num; i++) {
		mod = &mods[i];
		if (!strcmp(mod->name,"mod_apache") || !strcmp(mod->name,"mod_cpu") || !strcmp(mod->name,"mod_mem") || !strcmp(mod->name,"mod_load") || !strcmp(mod->name,"mod_partition") || !strcmp(mod->name,"mod_io") || !strcmp(mod->name,"mod_tcp") || !strcmp(mod->name,"mod_traffic") || !strcmp(mod->name,"mod_nginx")) {
			mod->enable = 1;
		} else
			mod->enable = 0;
	}   
}
/*end*/
#endif

void init_module_fields(void) {
    struct module *mod;
    int i;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->enable)
            continue;

        // 合并模式设 mod->n_item为1
        if (MERGE_ITEM == conf.print_mode)
            mod->n_item = 1;
        else {
            // 项以分号分隔,多个分号表示多个项
            mod->n_item = get_strtok_num(mod->record, ITEM_SPLIT);
            // mod至少有一个项
            if (!mod->n_item)
                mod->n_item = 1;
        }

        if (mod->n_item) {
            /* 分配项数乘以列数个U_64大小的内存空间 */
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

/* read line from file to mod->record */
void read_line_to_module_record(char *line) {
    int i;
    struct module *mod;
    char *s_token, *e_token;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (mod->enable) {
            memset(mod->record, 0, sizeof(mod->record));
            if(!(s_token = strstr(line, mod->opt_line)))
                continue;
            s_token += strlen(mod->opt_line) + sizeof(STRING_SPLIT) - 1;
            if ((e_token = strstr(s_token, SECTION_SPLIT)))
                memcpy(mod->record, s_token, e_token - s_token);
            else
                memcpy(mod->record, s_token, strlen(line) - (s_token - line));
        }
    }
}

/* 根据n_item值来分配内存空间 */
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
            mod->pre_array = (U_64 *)calloc(n_n_item * mod->n_col, sizeof(U_64));
            mod->cur_array = (U_64 *)calloc(n_n_item * mod->n_col, sizeof(U_64));
            mod->st_array = (double *)calloc(n_n_item * mod->n_col, sizeof(double));
            if (conf.print_tail) {
                mod->max_array = (double *)calloc(n_n_item * mod->n_col, sizeof(double));
                mod->mean_array = (double *)calloc(n_n_item * mod->n_col, sizeof(double));
                mod->min_array = (double *)calloc(n_n_item * mod->n_col, sizeof(double));
            }
        }
    }
}

/*
 * set st result in st_array
 */
void set_st_record(struct module *mod) {
    int i, j, k = 0;
    struct mod_info *info = mod->info;
    mod->st_flag = 1;

    for (i = 0; i < mod->n_item; i++) {
        /* set module record by yourself */
        if (mod->set_st_record) {
            mod->set_st_record(mod, &mod->st_array[i * mod->n_col],
                    &mod->pre_array[i * mod->n_col],
                    &mod->cur_array[i * mod->n_col],
                    conf.print_interval);
        }

        for (j = 0; j < mod->n_col; j++) {
            /* 有set module record 不执行这里 */
            if (!mod->set_st_record) {
                printf("not set_cpu_record\n");
                /* cur_array与pre_array保留最大的 */
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
                if (0 == mod->n_record)
                    mod->max_array[k] = mod->mean_array[k] = mod->min_array[k] = mod->st_array[k] * 1.0;
                else {
                    if (mod->st_array[k] - mod->max_array[k] > 0.1)
                        mod->max_array[k] = mod->st_array[k];
                    if (mod->min_array[k] - mod->st_array[k] > 0.1 && mod->st_array[k] >= 0)
                        mod->min_array[k] = mod->st_array[k];
                    if (mod->st_array[k] >= 0)
                        mod->mean_array[k] = ((mod->n_record - 1) * mod->mean_array[k] + mod->st_array[k]) / mod->n_record;
                }
            }

            k++;
        }
    }

    mod->n_record++;
}

/*
 * 程序流程
 * 1.循环每个module
 * 2.得到mod->record中有多少项目
 * 3.不为合并模式且得到的项目数和mod->n_item不一样则重新分配内存空间
 * 4.在mod->record里查找分号
 *  4.1 多项目: 
 *      合并模式下合并多个项目
 *      非合并模式下合并多个项目
 *  4.2 单项目: mod->record内容以10进制形式保存到mod->cur_array里
 */
int collect_record_stat(void) {
    int i, n_item, ret, no_p_hdr = 1;
    struct module *mod = NULL;
    U_64 *tmp;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->enable)
            continue;

        mod->st_flag = 0;
        ret = 0;

        // mod->record项目数
        if ((n_item = get_strtok_num(mod->record, ITEM_SPLIT))) {
            /* not merge mode and last n_item != cur n_item */
            if (MERGE_ITEM != conf.print_merge && n_item && n_item != mod->n_item) {
                no_p_hdr = 0;
                /* 根据n_item值来分配内存空间 */
                realloc_module_array(mod, n_item);
            }

            mod->n_item = n_item;

            /* multiply items */
            if (strstr(mod->record, ITEM_SPLIT)) {
                /* merge mode */
                if (MERGE_ITEM == conf.print_merge) {
                    mod->n_item = 1;
                    /* 合并模式下,合并多个项目 */
                    ret = merge_mult_item_to_array(mod->cur_array, mod);
                } else { /* Not merge mode,merge mulitiply items*/
                    char item[LEN_128] = {0};
                    int num = 0;
                    int pos = 0;

                    while (strtok_next_item(item, mod->record, &pos)) {
                        /* io: 前11位保存第一项数据,后11位保存第二项数据,依此类推 */
                        if (!(ret = convert_record_to_array(&mod->cur_array[num * mod->n_col], mod->n_col, item)))
                            break;
                        memset(item, 0, sizeof(item));
                        num++;
                    }
                }
            } else  /* one item */
                ret = convert_record_to_array(mod->cur_array, mod->n_col, mod->record);

            /* get st record */
            if (no_p_hdr && mod->pre_flag && ret)
                set_st_record(mod);

            /* ret为假,mod->pre_flag也要为假 */
            if (!ret)
                mod->pre_flag = 0;
            else
                mod->pre_flag = 1;

        } else 
            mod->pre_flag = 0;

        /* swap cur_array to pre_array */
        tmp = mod->pre_array;
        mod->pre_array = mod->cur_array;
        mod->cur_array = tmp;
    }

    return no_p_hdr;
}

/* 当module列数为0,则置module enable为0 */
void disable_col_zero(void) {
    struct module *mod = NULL;
    int i, j;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->enable)
            continue;
        if (!mod->enable)
            mod->enable = 0;
        else {
            struct mod_info *info = mod->info;
            int p_col = 0;

            for (j = 0; j < mod->n_col; j++) {
                if (((DATA_SUMMARY == conf.print_mode) && (SUMMARY_BIT == info[j].summary_bit))
                        || ((DATA_DETAIL == conf.print_mode) && (HIDE_BIT != info[j].summary_bit))) {
                    p_col++;
                    break;
                }
            }

            if (!p_col)
                mod->enable = 0;

        }
    }
}

/* free module info */
void free_modules(void) {
    int i;
    struct module *mod;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (mod->lib)
            dlclose(mod->lib);

        if (mod->cur_array) {
            free(mod->cur_array);
            free(mod->pre_array);
            free(mod->st_array);
            mod->cur_array = NULL;
            mod->pre_array = NULL;
            mod->st_array = NULL;
        }

        if (mod->max_array) {
            free(mod->max_array);
            free(mod->mean_array);
            free(mod->min_array);
            mod->max_array = NULL;
            mod->mean_array = NULL;
            mod->min_array = NULL;
        }
    }
}
