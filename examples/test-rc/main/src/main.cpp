#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>

#include "hg_rc.h"
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
    int rc_fd = -1;
    uint8_t rbuf[128];

    rc_fd = uart_open(RC_PORT, RC_BUNDRATE);
    if(rc_fd != -1) {			
		printf("[RC] RC uart: ready!\n");
	}

    while (1)
    {
        int s_ret = read(rc_fd, rbuf, 128);
        //printf("[device_server]%d \n", s_ret);
        for (int i=0; i<s_ret; i++) {
            bool ret = ibus_decoder(rbuf[i]);  
            if(ret == true) {
                uint16_t rc_ch[8];
                ibus_get_values(rc_ch, 8);
                printf("[RC] rc: %d %d %d %d %d %d %d %d\n",
                   rc_ch[0],rc_ch[1],rc_ch[2],rc_ch[3],rc_ch[4],rc_ch[5],rc_ch[6],rc_ch[7]); 
            }
        } 			
        usleep(10*1000); 
    }

    return 0;
}

