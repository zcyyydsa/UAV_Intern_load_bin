#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>

#include "hg_ptz.h"
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
    int ptz_fd = -1;
    uint8_t sbuf[128];
    uint8_t rbuf[128];
    ptz_payload_rx_data_status_t ptz_data = {0};

    ptz_fd = uart_open(PTZ_DEV, 460800);
    if(ptz_fd != -1) {			
		printf("[PTZ] PTZ uart: ready!\n");
	}

    int size = ptz_pack_pitch_to_buffer(-3000, sbuf); // -3000(-30 degree/s)
    write(ptz_fd, sbuf, size);
    sleep(1);
    size = ptz_pack_pitch_to_buffer(0, sbuf);
    write(ptz_fd, sbuf, size);

    while (1)
    {
        int s_ret = read(ptz_fd, rbuf, 128);
        //printf("%d \n", s_ret);
        for (int i=0; i<s_ret; i++) {
            int ret = ptz_parse_char(rbuf[i], &ptz_data);  
            if(ret == 1) {
                printf("[PTZ] pitch=%d", ptz_data.pitch);
            }
        }		
        usleep(10*1000); 
    }

    uart_close(ptz_fd);
    return 0;
}

