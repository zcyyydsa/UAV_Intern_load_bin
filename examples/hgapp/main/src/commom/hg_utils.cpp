#include "hg_utils.h"

#include <sys/path.h>
#include <port/system.h>

#include <time.h>
#include <dirent.h> // for get_file_lists

#include <sys/time.h>

int check_dir_exist(const char* path)
{
	int ret = -1;
	ret = path_testdir(path);
    if (ret == 0) {
        printf("%s no exist.", path);
        ret = path_makedir(path);
        if (ret == 0) {
            printf("create %s ok.", path);
        } else {
            printf("create %s err.", path);
        }
    }

    return 0;
}

int get_file_lists(const char* media_path, const char* json_path)
{
    struct dirent *ent = NULL;
    struct stat fileinfo;
    int sum = 0;
    int ret = 0;

    struct dirent **namelist;
    char filepath[128];
    int file_cnt;
    int i = 0, j = 0;
	char json_buf[128*1024] = {0};
	struct tm* stime;

    // ret = storage_device_is_exist(SD_DEV);
    // if (ret == 0) {
    //     SPDLOG_ERROR("open lists.json err, sd not exist.");
    //     return -1;
    // }

	FILE* fp = fopen(json_path, "w");
	if (!fp) {
		// SPDLOG_ERROR("open lists.json err.");
		return -1;
	}

    file_cnt = scandir(media_path, &namelist, 0, alphasort);
    if (file_cnt < 0) {
        return -1; 
    } else { 
        sum = file_cnt - 2; // ignore . and ..
        //SPDLOG_INFO("file cnt={}", sum);

		strcat(json_buf, "{\"file_list\"");
		strcat(json_buf,":[");
        while (i < file_cnt)
        {
            ent = namelist[i];
            //if (ent->d_type == 8) {
                snprintf(filepath, sizeof(filepath), "%s/%s", media_path, ent->d_name);
                if (stat(filepath, &fileinfo) < 0) {
                    //SPDLOG_ERROR("stat({:s})\n", filepath);
                    return -1;
                }

                if ((ent->d_name[0] == '.') || (ent->d_name[0] == '..')) {
                    free(namelist[i]);
                    i++;
                    continue;
                }
               
				strcat(json_buf, "{\"path\":\"");
				strcat(json_buf, filepath);
				strcat(json_buf, "\",");
				strcat(json_buf, "\"thumb_path\":\"");
				strcat(json_buf, " "); // TODO
				strcat(json_buf, "\",");  

				stime = localtime(&fileinfo.st_mtime);
				sprintf(&json_buf[strlen(json_buf)],"\"date\":\"%04d-%02d-%02d", stime->tm_year + 1900,stime->tm_mon + 1, stime->tm_mday);
				sprintf(&json_buf[strlen(json_buf)]," %02d:%02d:%02d\",", stime->tm_hour,stime->tm_min, stime->tm_sec);
				sprintf(&json_buf[strlen(json_buf)], "\"size\":\"%.2fMB\",", (float) (fileinfo.st_size*100/(1024*1024))/100);
				strcat(json_buf, "\"name\":\"");
				strcat(json_buf, ent->d_name);
				strcat(json_buf, "\"");
				strcat(json_buf, "},");

            //}
            free(namelist[i]);
            i++;
        }
		sprintf(&json_buf[strlen(json_buf)-1],"]}");
        free(namelist);
//printf("json: %s\n", json_buf);
		fwrite(&json_buf[0], 1, strlen(json_buf), fp);
    }

	fclose(fp);
    //m_avList->count = j;

    return 0;
}

int sys_set_time(uint64_t utc)
{
    // struct tm _tm;
    struct timeval tv;
	// time_t utc;
	int ret;

    // _tm.tm_sec = msg->second;
    // _tm.tm_min = msg->minute;
    // _tm.tm_hour = msg->hour;
    // _tm.tm_mday = msg->day;
    // _tm.tm_mon = msg->month - 1;
    // _tm.tm_year = msg->year - 1900;
	// _tm.tm_isdst = -1;

    // utc = mktime(&_tm);
	// if (utc==-1) {
	// 	printf("Set system datetime mktime err!");
	// }                           
    tv.tv_sec = utc;
    tv.tv_usec = 0;

	//printf("%d%d%d %d%d%d %lld\n", _tm.tm_year,_tm.tm_mon,_tm.tm_mday,_tm.tm_hour,_tm.tm_min,_tm.tm_sec,utc);
    //if(settimeofday(&tv, (struct timezone *) 0) < 0)
	//if(settimeofday(&tv, &tz) < 0)
	ret = settimeofday(&tv, NULL);
	if (ret < 0) {      
        return 1;
    }   
    return 0;
}