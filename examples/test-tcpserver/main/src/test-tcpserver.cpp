/**
 * @file main.cpp
 * @author gwl
 * @brief communication interface between drone and upper computer test
 * @version 1.0 
 * @date 2023/6/12
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "hg_net_protocol.h"


static NetCon *g_netCon;
static void sigterm_handler(int sig) {
  fprintf(stderr, "signal %d\n", sig);
  g_netCon->run = false;
  printf("signal_run:%d\n", g_netCon->run);
}

static int net_open(NetCon *nc, void *data)
{
   memset(data, 0, sizeof(data));
   printf("open \n");

   return 0;
}

static int net_close(NetCon *nc, void *data)
{
    memset(data, 0, sizeof(data));
    printf("close \n");

    return 0;
}

static int net_write(NetCon *nc, void *data)
{
    uint8_t *wdata = (uint8_t *)data;
    int ret = ncio_snd(nc, wdata, LEN);   
    printf("srv send:%s\n", wdata);

    return 0;
}

static int net_read(NetCon *nc, void *data) 
{
    uint8_t *rdata = (uint8_t *)data;
    
    int ret;
    ret =  ncio_rcv(nc, rdata, LEN); 
    printf("srv recv:%s\n", rdata);

    return ret;
}


int main()
{
    signal(SIGINT, sigterm_handler);
    g_netCon = net_connect_init(net_open, net_close, net_read, net_write);

    while(g_netCon->run) {
      sleep(1);
    }
    if(g_netCon != NULL) {
      net_uninit(g_netCon);
    }

    return 0;
}
