#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "cfginfo.h"
#include "Setting.h"
#include "sharemem.h"
#include "main.h"
//#include "EEPROM.h"
#include "version.h"

extern SYSTEM_ATTR_s g_system_attr;


#if 1
static int get_random(void)
{
	struct timeval tv;
    gettimeofday(&tv,NULL);
	
	//printf("(unsigned int)time(NULL) : %d \n", (unsigned int)time(NULL));
	srand((unsigned int)(tv.tv_sec*1000000 + tv.tv_usec));
	
	return rand();
}
#endif

int AppInitCfgInfoDefault(void)
{
    //RUN_STATUS.ucHDMIStatus = HDMI_OUT;
    share_mem->sm_run_status.ucInputStatus = NONE;
    share_mem->sm_run_status.usWidth = 0;
    share_mem->sm_run_status.usHeight = 0;
    share_mem->sm_run_status.ucFrameRate = 0;
    share_mem->sm_run_status.usAudioSampleRate = 0;

#if 1
    share_mem->uuid = 0;
#endif
     
    strcpy(share_mem->sm_run_status.strHardwareVer, HD_VERSION);
    strcpy(share_mem->sm_run_status.strSoftwareVer, SW_VERSION);
    
    //ETH
    strcpy(share_mem->sm_eth_setting.strEthIp,"192.168.36.128");
    strcpy(share_mem->sm_eth_setting.strEthMask,"255.255.255.0");
    strcpy(share_mem->sm_eth_setting.strEthGateway,"192.168.36.3");        
    strcpy(share_mem->sm_eth_setting.strEthMulticast,"239.255.42.1");
    
    //ENCODER
    share_mem->sm_encoder_setting.usEncRate = 1000;
    share_mem->sm_encoder_setting.ucInterval = 30;
    share_mem->sm_encoder_setting.ucIQP = 36;
    share_mem->sm_encoder_setting.ucPQP = 40;
    share_mem->sm_encoder_setting.ucFrameRate = 30;    
    
    //CEC CODE
    strcpy(share_mem->cec_control.cec_code_on, "40-04");
    strcpy(share_mem->cec_control.cec_code_off, "ff-36");
    share_mem->cec_control.cec_state = 0;
    
    return 1;
}

int AppInitCfgInfoFromFile(int *fp)
{
	int iRetCode = 0;
	char strTemp[20];

    printf("open config file \n");
	*fp = open(CONFIG_FILE, O_RDONLY);
	printf("fp = %d, *fp = %d\n",fp,*fp);
	if(*fp < 0)
	{
		share_mem->uuid = get_random();
		printf("%s open failed\n",CONFIG_FILE);
		return -1;
	}
    printf("open config file ok \n");

    printf("get eth ip \n");
    printf("share_mem->sm_eth_setting.strEthIp = %s \n", share_mem->sm_eth_setting.strEthIp);
	iRetCode = GetConfigStringValue(*fp,"ETH","ETH_IP",share_mem->sm_eth_setting.strEthIp);  
	printf("share_mem->sm_eth_setting.strEthIp = %s \n", share_mem->sm_eth_setting.strEthIp);
	if(iRetCode)
	{
		printf("get Eth IP failed, %d !\n", iRetCode); 
		return -2;
	}

    printf("get eth ip ok\n");
	iRetCode = GetConfigStringValue(*fp,"ETH","ETH_MASK",share_mem->sm_eth_setting.strEthMask);  
	iRetCode = GetConfigStringValue(*fp,"ETH","ETH_GATEWAY",share_mem->sm_eth_setting.strEthGateway);  
	iRetCode = GetConfigStringValue(*fp,"ETH","ETH_MULTICAST",share_mem->sm_eth_setting.strEthMulticast);  

#if 1 //uuid
	iRetCode = GetConfigStringValue(*fp,"ETH","UUID",strTemp);
	share_mem->uuid = atoi(strTemp);
#endif
    //iRetCode = GetConfigStringValue(*fp,"ETH","CEC_ON",share_mem->cec_control.cec_code_on);
    //iRetCode = GetConfigStringValue(*fp,"ETH","CEC_OFF",share_mem->cec_control.cec_code_off);

	return 0;
}

int AppWriteCfgInfotoFile(void)
{
    printf("\n-------AppWriteCfgInfotoFile----------\n");
#ifdef CONFIG_EEPROM
    if (TRUE == g_system_attr.e2prom)
    {
        printf("EEPROM write start \n");
        WriteConfigIntoE2prom(share_mem);
        return 0;
    }
#endif
	int iRetCode = 0; 
	char strTemp[20];
    FILE* fp;
    fp = fopen(CONFIG_FILE, "w");
    printf("write conf file start\n");
    
    //Section Eth
    fprintf(fp,"[ETH]\n");
    fprintf(fp,"ETH_IP=%s\n",share_mem->sm_eth_setting.strEthIp);
    fprintf(fp,"ETH_MASK=%s\n",share_mem->sm_eth_setting.strEthMask);
    fprintf(fp,"ETH_GATEWAY=%s\n",share_mem->sm_eth_setting.strEthGateway);
    fprintf(fp,"ETH_MULTICAST=%s\n",share_mem->sm_eth_setting.strEthMulticast);
    
	//UUID
    fprintf(fp,"UUID=%d\n",share_mem->uuid);

    //CEC CODE 
    //fprintf(fp,"CEC_ON=%s\n",share_mem->cec_control.cec_code_on);
    //fprintf(fp,"CEC_OFF=%s\n",share_mem->cec_control.cec_code_off);
    
    fprintf(fp,"[END]");
    
    fclose(fp);
    printf("write conf file finish\n");
    return 0;
}
