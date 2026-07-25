// Simple GUI от RA4ASN

#ifndef _gui_utils_h
#define _gui_utils_h

#include "gui_user_include.h"

int safe_strcmp(const char * s1, const char * s2);
int is_valid_datetime(int year, int month, int day, int hour, int minute, int second);
void remove_end_line_spaces(char * str);
const char * remove_start_line_spaces(const char * str);
int snormalize(int raw, int rawmin, int rawmax, int range);

#endif /* _gui_utils_h */
