/**
 * @file udp_client.h
 * @author gwl
 * @brief  udp broadcast info to server and recv server ip and ports
 * @version 1.0
 * @date 2023/6/29
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */

#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>

typedef struct {
  int sock;
  int addr_len;
  int port = 0;
  std::string ip;
  struct sockaddr_in broadcast_addr;
  struct sockaddr_in server_addr;
  bool isRunning = false;
}UdpNetWork;

extern UdpNetWork unet;
int hg_udp_clt_init(std::string &ip, int &port);
void hg_udp_clt_uninit(UdpNetWork *nw);

#endif // !UDP_CLIENT_H

