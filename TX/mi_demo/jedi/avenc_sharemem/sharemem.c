#include <stdlib.h>
#include <string.h>
#include "sharemem.h"
#include "linux_common.h"
#include "cfginfo.h"
#include "version.h"

char web_flag;
char g_backup_flag;
extern unsigned char g_Exit;
extern char multicast[20];

int InitShareMem(void)
{
	int shmid;
	int i; 
	
	shmid = GetShareMemoryID((key_t)1234, sizeof(SHARE_MEM), 0666|IPC_CREAT);	
	if(shmid < 0)  
	{
		exit(0);
	}

	share_mem = (struct shared_use_st*)AttacheShareMemoryToProcess(shmid, (void*)0, 0);  
	if(share_mem < 0)
	{
		exit(0);
	}
	printf("Memory attached at %X\n", (int)share_mem);

    share_mem->sm_eth_setting = eth_setting;
	AppInitCfgInfoDefault();
}

static int print_sharememary()
{
	printf("IP: %s \n",share_mem->sm_eth_setting.strEthIp);
	printf("Mask: %s \n",share_mem->sm_eth_setting.strEthMask);
	printf("Gateway : %s \n", share_mem->sm_eth_setting.strEthGateway);
	printf("Multicast : %s \n", share_mem->sm_eth_setting.strEthMulticast);
	printf("ucInfoDisplayFlag : %d \n", share_mem->ucInfoDisplayFlag);
	return 0;
}

void sharemem_main(void)
{
	int ret;
	
	ret = IsExistent(CONFIG_FILE);
	if (ret < 0)
	{
		AppWriteCfgInfotoFile();
		AppWriteModeInfotoFile();
	}
	else
	{
		AppInitCfgInfoFromFile();
	}

	while(g_Exit)
	{
		//print_sharememary();
		if(1==share_mem->ucUpdateFlag)
		{
			printf("\n\n--------web start -------\n");
			AppWriteCfgInfotoFile();
#if 1
			strcpy(multicast, share_mem->sm_eth_setting.strEthMulticast);
#endif
			web_flag = 1;
			share_mem->ucUpdateFlag = 0;
			g_backup_flag = 1;
			#if 0
			for (int i=0; i<128; i++)
			{
				printf("Multiaddress[%d] : %d\n", i, share_mem->sm_group_pack.ucMultiAddress[i]);
			}
			#endif
			printf("control flag : %d \n", share_mem->ucControlBoxFlag);
			printf("\n\n--------------web end------------- \n");
		}
		if (1==share_mem->ucModeApplyFlag)
		{
			printf("\n\n--------mode apply -------\n");
			AppWriteModeInfotoFile(); //mode 
			AppInitCfgInfoFromFile(); //flush current share memory
			share_mem->ucModeApplyFlag = 0;
		}
		sleep(1);
	}
	printf("-------------- sharememory thread exit ----------------\n");
	return NULL;
}
