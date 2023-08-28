#include <stdio.h>
#include <unistd.h>

#include "hg_flightctrl.h"

uint8_t buf[256];
void* get(void* param, int bytes)
{
    return buf;
}
void release(void* param, void* packet)
{

}
void onrecv(void* param, const void* packet, uint16_t bytes, int msgid)
{
    printf("msg id = %d\n", msgid);
}


int main()
{
    flightctrl_hdr_t hdr = {get,release,onrecv,};

    hgdf::flightctrl_create();
    hgdf::flightctrl_register(&hdr);

    hgdf::flightctrl_unlock();
    sleep(5);
    hgdf::flightctrl_lock();

    getchar();

    hgdf::flightctrl_destroy();

    return 0;
}


/*
test msg: heartbeat id 0
FE 87 00 00 00 E4 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF 00 00 05 00 00 12 40 FE 0F 01 00 00 00 00 00 00 00 00 00 00 00 09 00 00 00 00 00 00 88 95
*/
