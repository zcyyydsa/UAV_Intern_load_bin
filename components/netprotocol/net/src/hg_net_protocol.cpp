/**
 * @file hg_net_protocol.cpp
 * @author gwl
 * @brief communication interface between drone and upper computer
 * @version 1.0
 * @date 2023/6/12
 * 
 * Copyright (c) 2023 All rights reserved.
 * 
 */

#include "hg_net_protocol.h"

void sys_error(NetCon *nc) {
  nc->run = false;
	free(nc);
	nc = NULL;
}

static void *net_data_listen(NetCon *nc) {
  pthread_detach(pthread_self());
  int fd, cfd, maxfd, i, rlen = 0;
  int ret,res;
  fd_set g_fd, c_fd;
  uint8_t rdata[128] = {0};

  FD_ZERO(&g_fd);
  FD_SET(nc->sfd, &g_fd);
  maxfd = nc->sfd;

  while(nc->run) {
	c_fd = g_fd;
	res = select(maxfd+1, &c_fd, NULL, NULL, 0);
	if(res < 0) {
	  continue;
	}else if(res > 0){
	  for(i=0;i<=maxfd;i++) {
		  if(FD_ISSET(i, &c_fd)) { /*there are some fd touched */
		     /* new fd join in*/
		     if(i == nc->sfd) { 
		 	     cfd = accept(nc->sfd, NULL, NULL);
			     FD_SET(cfd, &g_fd);
			     maxfd = maxfd > cfd? maxfd : cfd;
		     }else { /* client send msg to us */
		       nc->cfd = i;
			     rlen = nc->nc_read(nc, &rdata);
           if(rlen <= 0){
				     close(i);
				     FD_CLR(i, &g_fd);
			     }else {
             nc->nc_write(nc, &rdata);
           }
		     }
		   }
	  }
	}else {
		printf("timeout\n");
	}
  }
  printf("thread is quit\n");
}

NetCon *net_connect_init(int( *nc_open)(NetCon *nc, void *data), 
                       int( *nc_close)(NetCon *nc, void *data),
                       int( *nc_read)(NetCon *nc, void *data), 
                       int( *nc_write)(NetCon *nc, void *data)) {
  NetCon *nc;
  struct sockaddr_in addr;
  nc = (NetCon *)malloc(sizeof (NetCon));
  if (NULL == nc) {
    return NULL;
  } 

  nc->sfd = socket(AF_INET, SOCK_STREAM, 0);
  if( nc->sfd < 0) {
  	perror("socket create failed, please try again\n");
  	sys_error(nc);	
	  return nc;	
  }
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(PORT);

  if(bind(nc->sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind error\n");
    close(nc->sfd);
    sys_error(nc);
	  return nc;
  }

  if(-1 == listen(nc->sfd, 1)) {
    perror("listen error\n");
    close(nc->sfd);
    sys_error(nc);
		return nc;
  }

  nc->nc_open = nc_open;
  nc->nc_close = nc_close;
  nc->nc_read = nc_read;
  nc->nc_write = nc_write;
  nc->run = true;
  nc->cfd = 0;
	nc->con_cnt = 0;
  if(0 != pthread_create(&nc->thread, NULL, (void *(*)(void *))net_data_listen, nc)) {
	    perror("net thread create failed\n");
      sys_error(nc);
	    return nc;
  }
  return nc;
}

int ncio_snd(NetCon *nc, void *data, int len) {
 // uint8_t *ncs_data = (uint8_t *)data;
  //printf("srv send:%02x\n", ncs_data[0]);
  return send(nc->cfd, data, len, MSG_NOSIGNAL);
}

int ncio_rcv(NetCon *nc, void *data, int len) {
  return recv(nc->cfd, data, len, 0);
}

void net_uninit(NetCon *nc)
{
  nc->run = false;
  sleep(1);
  close(nc->cfd);
  close(nc->sfd);
  free(nc);
    
  return;
}
