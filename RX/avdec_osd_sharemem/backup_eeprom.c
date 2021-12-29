#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>

#include "sharemem.h"
#include "backup_eeprom.h"


#ifdef MUTEX_IIC
extern pthread_mutex_t mutex_iic;
#endif

static char new_strSoftwareVer[20];
static char old_strSoftwareVer[20];

int InitShareMemFromE2prom(SHARE_MEM *pShareMemory)
{
	int ret = 0;

	init_eeprom();
#ifdef MUTEX_IIC
	pthread_mutex_lock(&mutex_iic);
#endif
	//SHARE_MEM *pRead = malloc(sizeof(SHARE_MEM) * sizeof(char));
	
	printf("buffer size : %d \n", sizeof(SHARE_MEM));
    printf("uuid: %d \n", pShareMemory->uuid);
	printf("ip : %s \n", pShareMemory->sm_eth_setting.strEthIp);
	printf("gateway : %s \n", pShareMemory->sm_eth_setting.strEthGateway);
	printf("multicast : %s \n", pShareMemory->sm_eth_setting.strEthMulticast);

    strcpy(new_strSoftwareVer,share_mem->sm_run_status.strSoftwareVer);
	
	printf("new_soft: %s\n",share_mem->sm_run_status.strSoftwareVer);
    
	sleep(1);
	
	ret = i2c_read_within_chip(0x00, pShareMemory, sizeof(SHARE_MEM));
	if (ret >= 0)
	{
		printf("\n========== eeprom read finish ==========\n");
		printf("uuid: %d \n", pShareMemory->uuid);
		printf("ip : %s \n", pShareMemory->sm_eth_setting.strEthIp);
		printf("gateway : %s \n", pShareMemory->sm_eth_setting.strEthGateway);
		printf("multicast : %s \n", pShareMemory->sm_eth_setting.strEthMulticast);

		if (0 != strncmp(pShareMemory->sm_eth_setting.strEthMask, "255.255.255.0", 13))
		{
			printf("\n+++++++++++ eeprom data error ++++++++++++++\n");
			ret = 2;
		}
		else
		{
			printf("\n+++++++++++ eeprom data ok +++++++++++++++\n");
			ret = 1;
            strcpy(old_strSoftwareVer,share_mem->sm_run_status.strSoftwareVer);
	        printf("old_soft: %s\n",share_mem->sm_run_status.strSoftwareVer);

		    ret = strcmp(new_strSoftwareVer,old_strSoftwareVer);
            if(ret != 0)
            {
                strcpy(share_mem->sm_run_status.strSoftwareVer,new_strSoftwareVer);
                ret = WriteConfigIntoE2prom(share_mem);
                printf("writeconfigintoe2prom ret : %d\n",ret);

            }
        }
	}
	else
	{
		if (-2 == ret) //first read eeprom
		{
			printf("first read eeprom\n");
			i2c_write_within_chip(0x00, pShareMemory, sizeof(SHARE_MEM));
		}
		printf("\n---------- eeprom read failed ----------\n");
		ret = -1;
	}
#ifdef MUTEX_IIC
	pthread_mutex_unlock(&mutex_iic);
#endif
	deinit_eeprom();
	return ret;
}

int WriteConfigIntoE2prom(SHARE_MEM *pShareMemory)
{
	init_eeprom();
	int ret = 0;
#ifdef MUTEX_IIC
	pthread_mutex_lock(&mutex_iic);
#endif
	printf("\n------ eeprom write size: %d -------\n", sizeof(SHARE_MEM));
	printf("\n------ eeprom write start --------\n");
	ret = i2c_write_within_chip(0x00, pShareMemory, sizeof(SHARE_MEM));
	printf("\n------ eeprom write finish -------- \n");
#ifdef MUTEX_IIC
	pthread_mutex_unlock(&mutex_iic);
#endif
	deinit_eeprom();
	return ret;
}
