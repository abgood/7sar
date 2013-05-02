#include "tsar.h"

struct configure conf;
struct module mods[MAX_MOD_NUM];
struct statistic statis;

void usage()
{
	int i;
	struct module *mod;

	fprintf(stderr,
			"Usage: tsar [options]\n"
			"Options:\n"
#ifdef OLDTSAR
			/*used for check alert*/
			"    -check		display last record for alert\n"
			//"    -current		display last record for alert\n"
			/*end*/
#endif
			"    --check/-C		display last record for alert.example:tsar --check / tsar --check --cpu --io\n"
			"    --cron/-c		run in cron mode, output data to file\n"
			"    --interval/-i	specify intervals numbers, in minutes if with --live, it is in seconds\n"
			"    --list/-L		list enabled modules\n"
			"    --live/-l		running print live mode, which module will print\n"
			"    --file/-f		specify a filepath as input\n"
			"    --ndays/-n		show the value for the past days (default: 1)\n"
			"    --date/-d		show the value for the specify day(n or YYYYMMDD)\n"
			"    --merge/-m		merge multiply item to one\n"
			"    --detail/-D	\tdo not conver data to K/M/G\n"
			"    --spec/-s		show spec field data, tsar --cpu -s sys,util\n"
			"    --help/-h		help\n");

	fprintf(stderr, "Modules Enabled:\n");

	for (i = 0; i < statis.total_mod_num; i++) {
		mod = &mods[i];
		if(mod->usage)
			fprintf(stderr, "%s\n", mod->usage);
	}

	exit(0);
}

struct option longopts[] = {
    {"cron", no_argument, NULL, 'c'},
    {"check", no_argument, NULL, 'C'},
    {"interval", required_argument, NULL, 'i'},
    {"list", no_argument, NULL, 'L'},
    {"live", no_argument, NULL, 'l'},
    {"file", required_argument, NULL, 'f'},
    {"ndays", required_argument, NULL, 'n'},
    {"date", required_argument, NULL, 'd'},
    {"merge", no_argument, NULL, 'm'},
    {"detail", no_argument, NULL, 'D'},
    {"spec", required_argument, NULL, 's'},
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0}
};

static void main_init(int argc, char **argv) {
    int opt, oind = 0;
#ifdef OLDTSAR
    if (argc >= 2) {
        if (!strcmp(argv[1], "-check") && argc == 2) {
            conf.running_mode = RUN_CHECK;
            conf.print_mode = DATA_DETAIL;
            conf.print_interval = 60;
            conf.print_tail = 0;
            conf.print_nline_interval = conf.print_interval;
            return;
        }
    }
#endif
    while ((opt = getopt_long(argc, argv, ":cCi:Llf:n:d:s:mhD", longopts, NULL)) != -1) {
        oind++;
        switch (opt) {
            case 'c':
                conf.running_mode = RUN_CRON;
                break;
            case 'C':
                conf.running_mode = RUN_CHECK_NEW;
                break;
            case 'L':
                conf.running_mode = RUN_LIST;
                break;
            case 'l':
                conf.running_mode = RUN_PRINT_LIVE;
                break;
            case 'i':
                conf.print_interval = atoi(optarg);
                oind++;
                break;
            case 'f':
                strcpy(conf.output_file_path, optarg);
                break;
            case 's':
                set_special_field(optarg);
                break;
            case 'n':
                conf.print_ndays = atoi(optarg);
                oind++;
                break;
            case 'd':
                conf.print_day = atoi(optarg);
                oind++;
                break;
            case 'm':
                conf.print_merge = MERGE_ITEM;
                break;
            case 'D':
                conf.print_detail = TRUE;
                break;
            case 'h':
                usage();
            case ':':
                printf("must have parameter\n");
                usage();
            case '?':
                if (argv[oind] && strstr(argv[oind], "--")) {
                    strcat(conf.output_print_mod, argv[oind]);
                    strcat(conf.output_print_mod, DATA_SPLIT);
                } else
                    usage();
        }
    }

    if (!conf.print_ndays)
        conf.print_ndays = 1;

    if (!conf.print_interval)
        conf.print_interval = DEFAULT_PRINT_INTERVAL;

    if (RUN_NULL == conf.running_mode)
        conf.running_mode = RUN_PRINT;

    if (conf.running_mode == RUN_CHECK_NEW) {
        conf.print_interval = 60;
        conf.print_tail = 0;
        conf.print_nline_interval = conf.print_interval;
    }

    if (!strlen(conf.output_print_mod))
        conf.print_mode = DATA_SUMMARY;
    else
        conf.print_mode = DATA_DETAIL;

    strcpy(conf.config_file, DEFAULT_CONF_FILE_PATH);
    if (access(conf.config_file, F_OK))
        do_debug(LOG_FATAL, "main_init: can't find tsar.conf");
}

void running_list() {
    int i;
    struct module *mod;

    printf("tsar enable follow modules:\n");

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        printf("\t%s\n", mod->name);
    }
}

void running_cron(void) {
    int have_collect = 0;

    if (strstr(conf.output_interface, "file")) {
        /* 一次收集数据,db和nagios都可以使用 */
        collect_record();
        output_file();
        have_collect = 1;
    }

    if (strstr(conf.output_interface, "db"))
        output_db();

    if (strstr(conf.output_interface, "nagios"))
        output_nagios();
}

int main (int argc, char **argv) {

    parse_config_file(DEFAULT_CONF_FILE_PATH);

    load_modules();

    statis.cur_time = time(NULL);

    conf.print_day = -1;

    main_init(argc, argv);

    switch (conf.running_mode) {
        case RUN_LIST:
            running_list();
            break;

        case RUN_CRON:
            conf.print_mode = DATA_DETAIL;
            running_cron();
            break;
#ifdef OLDTSAR            
        case RUN_CHECK:
            reload_check_modules();
            running_check(RUN_CHECK);
            break;
#endif        
        /*
        case RUN_CHECK_NEW:
            if (reload_modules(conf.output_print_mod))
                conf.print_mode = DATA_DETAIL;
            disable_col_zero();
            running_check(RUN_CHECK_NEW);
            break;
        */
    }

    return 0;
}
