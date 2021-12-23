#include "init_net.h"
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "gpio.h"


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
	printf("init eth : ");
    printf(syscmd);
    
    #if 1
    //Ip set
    system(syscmd);
    
    //multicast route address configure
    system(MUL_ADDRESS);
    system(DEFAULT_ROUTE);
    #endif
    printf("\ninit eth finish \n");

    return 0;
}

void * switch_ip(void)
{
    unsigned char tmp1, tmp2;
    char str[20] = {0};
    //char str_tmp[100] = {0};

    while (1)
    {
        tmp1 = detect_sw_id();

        #if 1
        if (tmp2 != tmp1)
        {
            tmp2 = tmp1;
            sprintf(str, "192.168.36.%d", tmp1+1);
            printf(str);
            if (strcmp(str, share_mem->sm_eth_setting.strEthIp))
            {
                strcpy(share_mem->sm_eth_setting.strEthIp, str);
                printf(share_mem->sm_eth_setting.strEthIp);
                AppWriteCfgInfotoFile();
                init_eth(); //ip configer
            }
        }
        #endif
        sleep(1);
    }
   return ;
}


