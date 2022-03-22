#include "sharemem.h"
#include <string.h>
#include "linux_common.h"
#include <stdlib.h>
//#include "EEPROM.h"
//#include "app_rx_io_ctl.h"
#include "main.h"
#include "version.h"

#define CONFIG_EEPROM

static int fd_config;

extern SYSTEM_ATTR_s g_system_attr;

int InitShareMem(void)
{
	int shmid;
	int ret = -1;

	printf("statr init share memory\n");
	shmid = GetShareMemoryID((key_t)1234, sizeof(SHARE_MEM), 0666|IPC_CREAT);	
	if (shmid < 0) exit(0);

	share_mem = (struct shared_use_st*)AttacheShareMemoryToProcess(shmid, (void*)0, 0);  
	if(share_mem < 0) exit(0);

	AppInitCfgInfoDefault(); //init share_mem to define data
#ifdef CONFIG_EEPROM
	ret = InitShareMemFromE2prom(share_mem); //check eeprom 
#else
	ret = -1;
#endif
	printf("ret : %d \n", ret);
	if (ret < 0) //isn't eeprom
	{
		AppInitCfgInfoDefault();
		g_system_attr.e2prom = 0;
		printf("\n\n***** e2prom read failed *****\n\n");
		ret = AppInitCfgInfoFromFile(&fd_config); //reinit share memory form file
		if (ret < 0)
		{
			if(NULL!=fd_config)
				close(fd_config);
			printf("build default config.conf \n");
			AppWriteCfgInfotoFile();
		}
		else
		{
			printf("cfg get from file \n");
			close(fd_config);
		}
	}
	else
	{
		g_system_attr.e2prom = 1;

		if (2 == ret)
		{
			AppInitCfgInfoDefault();
#ifdef CONFIG_EEPROM
			WriteConfigIntoE2prom(share_mem);
#endif
		}
	}
	printf("share_mem->sm_eth_setting.strEthIp : %s \n", share_mem->sm_eth_setting.strEthIp);
	printf("share_mem->sm_eth_setting.strEthMulticast : %s \n", share_mem->sm_eth_setting.strEthMulticast);
	printf("Memory attached at %X\n", (int)share_mem);
}

static void print_sharememory(void)
{
	printf("%s \n", share_mem->cec_control.cec_code_off);
	printf("%s \n", share_mem->cec_control.cec_code_on);
	printf("%s \n", share_mem->sm_eth_setting.strEthGateway);
	printf("%s \n", share_mem->sm_eth_setting.strEthIp);
	printf("%s \n", share_mem->sm_eth_setting.strEthMask);
	printf("%s \n", share_mem->sm_eth_setting.strEthMulticast);
}

void *sharemem_handle(void *arg)
{
	int ret;
	share_mem->ucUpdateFlag = 0;
	print_sharememory();
	while (1)
	{
		//printf("ucUpdateFlag=%d\n",share_mem->ucUpdateFlag);
		//print_sharememory();
		if (1==share_mem->ucUpdateFlag)
		{
			printf("\nweb start \n");
			strcpy(g_system_attr.multicast, share_mem->sm_eth_setting.strEthMulticast); //get now multicast address
			g_system_attr.multicast_change_flag = TRUE;
			printf("cec1 on code : %s \n", share_mem->cec_control.cec_code_on);
			printf("cec2 off code : %s \n", share_mem->cec_control.cec_code_off);
			printf("cec state : %d \n", share_mem->cec_control.cec_state);
			if (1 == share_mem->cec_control.cec_state) //CEC ON
			{
				UART_CEC_TV_ON();
			}
			else if (2 == share_mem->cec_control.cec_state) //CEC OFF
			{
				UART_CEC_TV_OFF();
			}
			share_mem->cec_control.cec_state = 0;
			AppWriteCfgInfotoFile();
			share_mem->ucUpdateFlag = 0;
			printf("\n\nweb end \n\n");
		}
		usleep(100000); //0.1s
	}
	return NULL;
}
