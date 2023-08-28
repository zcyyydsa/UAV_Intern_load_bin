#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>

#include "hg_tof.h"
#include "uart.h"

uint64_t get_time_usec()
{
	static struct timeval _time_stamp;
	gettimeofday(&_time_stamp, NULL);
	return _time_stamp.tv_sec*1000000 + _time_stamp.tv_usec;
}

int main()
{
    int ret = -1;
    int tof_fd = -1;
    uint8_t rbuf[10];
    tof_data_t tof_data = {0};

    tof_fd = uart_open(TOF_PORT, TOF_BUNDRATE);
    if(tof_fd != -1) {			
		printf("[TOF] TOF uart: ready!\n");
	}

    while (1)
    {
        int s_ret = read(tof_fd, rbuf, 10);
        //printf("[device_server]%d \n", s_ret);
        for (int i=0; i<s_ret; i++) {
            int ret = tof_parse_char(rbuf[i], &tof_data);  
            if(ret == 1) {
                printf("[TOF] tof height=%d stength=%d\n", tof_data.height, tof_data.strength);
                if (tof_data.strength > 100) { // tof height valid                           
                    printf("[TOF] tof height valid \n");
                } else { // tof height invalid
                    printf("[TOF] tof height invalid \n");
                }
            }
        } 			
        usleep(10*1000); 
    }

    uart_close(tof_fd);
    return 0;
}

