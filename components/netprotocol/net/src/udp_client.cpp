/**
 * @file udp_client.cpp
 * @author gwl
 * @brief udp broadcast info to server and recv server ip and ports
 * @version 1.0
 * @date 2023/6/29
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */

#include "udp_client.h"

#define IP_FOUND "IP_FOUND"
#define PORT 8888

UdpNetWork unet;
static void *udp_net_listen(UdpNetWork *net);

static void *udp_net_listen(UdpNetWork *net) {
  printf("udp thread is running\n");
  pthread_detach(pthread_self());
  int count,ret;
  fd_set readfd;
  char buffer[1024];
  net->port = 0;
  
  while(net->isRunning) {
    FD_ZERO(&readfd);
    FD_SET(net->sock, &readfd);

    ret = select(net->sock + 1, &readfd, NULL, NULL, NULL);

    if (ret > 0) {
      if (FD_ISSET(net->sock, &readfd)) {
        count = recvfrom(net->sock, buffer, 1024, 0, (struct sockaddr*)&(net->server_addr), (socklen_t *)&(net->addr_len));
        printf("recv:%s\n", buffer);
        if (strstr(buffer, IP_FOUND)) {
          net->ip = inet_ntoa(net->server_addr.sin_addr); 
          net->port = htons(net->server_addr.sin_port);
          printf("found server IP is %s, Port is %d\n", net->ip.c_str(), net->port);
        }else {
           printf("udp client recv broadcast info:%s\n", buffer);
          //printf("There is no get server ip and ports\n");
        }
      }
    }
  }
  printf("udp thread is ending\n");
  return NULL;
}

void hg_udp_clt_uninit(UdpNetWork *nw) {
  printf("hg udp client fd release\n");
  nw->isRunning = false;
  sleep(1);
  close(nw->sock);
}

int hg_udp_clt_init(std::string &ip, int &port) {
  
  int sd = 1;
  int ret;
  pthread_t uthread;
  printf("udp init\n");
  if(unet.isRunning == true) {
	  printf("udp is running, please disconnect it first\n");
	  return -1;
	}

  unet.sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (unet.sock < 0) {
    perror("sock error");
    hg_udp_clt_uninit(&unet);
    return -1;
  }
  printf("udp socket create success\n");
  unet.addr_len = sizeof(struct sockaddr_in);

  memset((void*)&(unet.broadcast_addr), 0, unet.addr_len);
  unet.broadcast_addr.sin_family = AF_INET;
  unet.broadcast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  unet.broadcast_addr.sin_port = htons(PORT);
  
   ret = bind(unet.sock, (struct sockaddr*)&unet.broadcast_addr, unet.addr_len);
   if (ret < 0) {
     perror("bind error\n");
     hg_udp_clt_uninit(&unet);
     return -1;
  }
  printf("udp bind success\n");
  if(unet.isRunning != true) {
    unet.isRunning = true;
	  if(0 != pthread_create(&uthread, NULL, (void *(*)(void *))udp_net_listen, &unet)) {
	    perror("client udp net thread create failed\n");
	    hg_udp_clt_uninit(&unet);
      return -1;
    }
  }
  ip = unet.ip;
  port = unet.port;

  return 0;
}
