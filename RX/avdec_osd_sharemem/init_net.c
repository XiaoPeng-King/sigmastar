#include "init_net.h"
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MUL_ADDRESS		"route add -net 224.0.0.0 netmask 240.0.0.0 dev eth0"
#define DEFAULT_ROUTE 	"route add default gw 192.168.36.1"

int init_eth(void)
{
    char syscmd[200];
    
    printf("\ninit eth start \n");
    strcpy(syscmd,"ifconfig eth0 ");
    strcat(syscmd,share_mem->sm_eth_setting.strEthIp);
    strcat(syscmd," netmask ");
    strcat(syscmd,share_mem->sm_eth_setting.strEthMask);
	
    printf(syscmd);
    system(syscmd);

    //multicast route address configure
    system(MUL_ADDRESS);
    system(DEFAULT_ROUTE);
    printf("\ninit eth finish \n");

    return 0;
}

