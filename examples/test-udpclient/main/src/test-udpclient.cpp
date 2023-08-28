#include "udp_client.h"

int main(int argc, char **argv)
{
    hg_udp_clt_init(unet.ip, unet.port);

	while(1) {
	  sleep(1);
	}
	hg_udp_clt_uninit(&unet); 
	return 0;
}
