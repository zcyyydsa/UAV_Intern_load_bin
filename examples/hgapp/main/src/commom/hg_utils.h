#ifndef __HG_UTILS_H_
#define __HG_UTILS_H_

#include <stdint.h>

int check_dir_exist(const char* path);
int get_file_lists(const char* media_path, const char* json_path);
int sys_set_time(uint64_t utc);

#endif