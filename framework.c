#include "tsar.h"

void register_mod_fileds(struct module *mod, char *opt, char *usage,
		struct mod_info *info, int n_col, void *data_collect, void *set_st_record)
{
	sprintf(mod->opt_line, "%s", opt);
	sprintf(mod->usage, "%s", usage);
	mod->info = info;
	mod->n_col = n_col;
	mod->data_collect = data_collect;
	mod->set_st_record = set_st_record;
}


void set_mod_record(struct module *mod, char *record)
{
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
