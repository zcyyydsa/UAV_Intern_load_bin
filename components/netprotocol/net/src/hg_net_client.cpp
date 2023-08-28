/**
 * @file hg_net_client.cpp
 * @author gwl
 * @brief tcp client communication interface between drone and upper computer test
 * @version 1.0 
 * @date 2023/6/12
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */

#include "hg_net_client.h"

#define PORT 8888

TcpNetWork tnet;
static void *tcp_net_listen(TcpNetWork *net);

void recv_print_data(hgprotocol_net_data *net_data) {
  //static int ret = 0;
  printf("client recv header:%02x\n", net_data->header);
  printf("client recv length:%02x\n", net_data->length);
  printf("client recv cmd:%02x\n",    net_data->cmd);
  printf("client recv sid:%02x\n",    net_data->start_id);
  printf("client recv eid:%02x\n",    net_data->end_id);
  printf("client recv pdata0:%02x\n", net_data->payload.data[0]);
  printf("client recv pdata1:%02x\n", net_data->payload.data[1]);
  printf("client recv pdata2:%02x\n", net_data->payload.data[2]);
  printf("client recv pdata3:%02x\n", net_data->payload.data[3]);
  printf("client recv crc:%02x\n",    net_data->crcSum);
  printf("------------------------------\n");
}

static void *tcp_net_listen(TcpNetWork *net) {
  printf("tcp thread is running\n");
  pthread_detach(pthread_self());
  int ret, rlen = 0;
  fd_set readfd;
  hgprotocol_net_data net_data;
  
  while(net->isRunning) {
    FD_ZERO(&readfd);
    FD_SET(net->fd, &readfd);

    ret = select(net->fd + 1, &readfd, NULL, NULL, NULL);
	if(0 == ret) {
		continue;
	}else if(ret > 0) {
      if (FD_ISSET(net->fd, &readfd)) {
        rlen = net_read(net->fd, &net_data, sizeof(net_data));  //recv data from server
		//recv_print_data(&net_data);
		hg_recv_net_data(&net_data);
      }  
	}else {
	  break;
	}
  }
  printf("tcp thread is disconnecting!\n");
  hg_net_clt_uninit(net);
  return NULL;
}

void hg_net_clt_uninit(TcpNetWork *nw) {
  printf("hg net client fd release\n");
  nw->isRunning = false;
  sleep(1);
  close(nw->fd);
    
  return;
}

int hg_net_clt_init(std::string ip, int port) {
	int ret;
	struct sockaddr_in addr;
	pthread_t thread;

	if(tnet.isRunning == true) {
	  printf("tcp is running, please disconnect it first\n");
	  return -1;
	}
	
	tnet.fd = socket(AF_INET, SOCK_STREAM, 0);
	if(tnet.fd < 0) {
	  perror("socket create failed, please try it again\n");
	  hg_net_clt_uninit(&tnet);
	  return -1;
	}
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	//addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	addr.sin_port = htons(port);

	ret = connect(tnet.fd, (struct sockaddr *)&addr, sizeof(addr));
	if(ret < 0) {
	  perror("socket connect failed, please try it again\n");
	  hg_net_clt_uninit(&tnet);
	  return -1;
	}
	if(tnet.isRunning != true) {
	  tnet.isRunning = true;
	  if(0 != pthread_create(&thread, NULL, (void *(*)(void *))tcp_net_listen, &tnet)) {
	    perror("client net thread create failed\n");
	    hg_net_clt_uninit(&tnet);
        return -1;
      }
	}else {
		//hg_net_clt_uninit(&tnet);
	}

	return 0;
}
