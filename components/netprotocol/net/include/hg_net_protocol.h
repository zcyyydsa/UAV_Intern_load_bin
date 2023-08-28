/**
 * @file hg_net_protocol.h
 * @author gwl
 * @brief communication interface between drone and upper computer
 * @version 1.0
 * @date 2023/6/12
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */

#ifndef HG_NET_PROTOCOL_H
#define HG_NET_PROTOCOL_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>

#define PORT   8888
#define LEN 32

typedef struct NetCon netCon;
struct NetCon {
  int sfd; // server fd
  int cfd; // client fd
  bool run; // run status
  int con_cnt; //connect count
  pthread_t thread; // thread handler
  
  int ( *nc_open)(NetCon *nc, void *data);  
  int ( *nc_close)(NetCon *nc, void *data);
  int ( *nc_read)(NetCon *nc, void *data);
  int ( *nc_write)(NetCon *nc, void *data);
};



/* sys error print */
void sys_error(NetCon *nc);

/* listen data from upper computer */
//static void *net_data_listen(NetCon *nc);

/* network connection init */
NetCon *net_connect_init(int ( *nc_open)(NetCon *nc,void *data), 
                       int ( *nc_close)(NetCon *nc, void *data),
                       int ( *nc_read)(NetCon *nc, void *data), 
                       int ( *nc_write)(NetCon *nc, void *data));

/* network data send callback */                       
int ncio_snd(NetCon *nc, void *data, int len);

/* network data reception callback */ 
int ncio_rcv(NetCon *nc, void *data, int len);

/* network uninit */
void net_uninit(NetCon *nc);


#endif // !HG_NET_PROTOCOL_H
