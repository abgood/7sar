#ifndef TSAR_COMMON_H
#define TSAR_COMMON_H

#define PRE_RECORD_FILE "/root/code/tsar/.tsar.tmp"

int get_st_array_from_file(int);
int get_strtok_num(char *, char *);
int merge_mult_item_to_array(U_64 *, struct module *);
int strtok_next_item(char item[], char *, int *);
int convert_record_to_array(U_64 *, int, char *);
void get_mod_hdr(char [], struct module *);

#endif
