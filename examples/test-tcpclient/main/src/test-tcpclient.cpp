#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define LEN 32
int flag = 0;
uint8_t sbuf[LEN] = "hello world";
uint8_t rbuf[LEN] = {0};
int main(int argc, char **argv)
{
	int fd,ret;
	struct sockaddr_in addr;
	
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd < 0)
	{
		perror("socket create failed, please try it again\n");
		return -1;
	}
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(PORT);

	ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
	if(ret < 0)
	{
		perror("socket connect failed, please try it again\n");
		return -1;
	}
	while(1)
	{
	  send(fd, sbuf, LEN, 0);
      printf("client_send:%s\n", sbuf);
	  bzero(rbuf, LEN);
	  int len = recv(fd, rbuf, LEN, 0);
	  printf("client_recv:%s\n", rbuf);
	  sleep(1);
	}
	close(fd);
	return 0;
}
