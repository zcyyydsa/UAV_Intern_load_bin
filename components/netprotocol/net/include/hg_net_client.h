/**
 * @file hg_net_client.h
 * @author gwl
 * @brief tcp client communication interface between drone and upper computer test
 * @version 1.0 
 * @date 2023/6/12
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */

#ifndef HG_NET_CLIENT_H
#define HG_NET_CLIENT_H

#include "hg_protocol.h"
//#include "udp_client.h"

typedef struct {
  int fd;
  bool isRunning = false;
}TcpNetWork;

template <typename T>
inline int net_send(int cfd, T *net_data, int len) {
  uint16_t sbuf[1024] = {0};
  memcpy(sbuf, net_data, len);
  return send(cfd, sbuf, len, MSG_NOSIGNAL);   
}

template <typename T>
inline int net_read(int cfd, T *net_data, int len) {
  uint16_t rbuf[1024] = {0}; 
  int rlen = recv(cfd, rbuf, sizeof(T), 0);
  memcpy(net_data, rbuf, rlen);
  return rlen;
}

extern TcpNetWork tnet;
void recv_print_data(hgprotocol_net_data *net_data);
int hg_net_clt_init(std::string ip, int port);
void hg_net_clt_uninit(TcpNetWork *nw);

#endif //!HG_NET_CLIENT_H
