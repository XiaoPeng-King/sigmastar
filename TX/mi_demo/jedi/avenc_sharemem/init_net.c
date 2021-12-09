#include "init_net.h"
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "gpio.h"

#define MUL_ADDRESS		"route add -net 224.0.0.0 netmask 240.0.0.0 dev eth0" 
#define DEFAULT_ROUTE 	"route add default gw 192.168.36.1"

static void switch_ip(void)
{
    char str[20] = {0};
    sprintf(str, "192.168.36.%d", IP_check());
    printf("\n\n-------");
    printf(str);
    printf("---------\n\n");
#if 0
    if (strcmp(str, share_mem->sm_eth_setting.strEthIp))
    {
        strcpy(share_mem->sm_eth_setting.strEthIp, str);
        printf(share_mem->sm_eth_setting.strEthIp);
        AppWriteCfgInfotoFile();
    }
#endif
}

int init_eth(void)
{
    char syscmd[200];
    
    switch_ip();

    strcpy(syscmd,"ifconfig eth0 ");
    strcat(syscmd,share_mem->sm_eth_setting.strEthIp);
    printf(syscmd);
    printf("share_mem->sm_eth_setting.strEthIp=%s\n",share_mem->sm_eth_setting.strEthIp);
    strcat(syscmd," netmask ");
    strcat(syscmd,share_mem->sm_eth_setting.strEthMask);
    system(syscmd);
    printf("ifconfig eth0=%s\n",syscmd);
    
    //multicast address configure
    system(MUL_ADDRESS);
    system(DEFAULT_ROUTE);
    
    return 0;
}
