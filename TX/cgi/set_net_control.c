#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Setting.h"
#include "sharemem.h"

int InitShareMem(void)
{

	int shmid;

	shmid = shmget((key_t)1234, sizeof(SHARE_MEM), 0666|IPC_CREAT);	
	if(shmid == -1)  
	{  
//		printf("shmget failed\n");  
        printf("fail \n");
		exit(0);
	}  

	share_mem = shmat(shmid, (void*)0, 0);  
	if(share_mem == NULL)  
	{ 
//		printf("shmat failed\n");	
        printf("fail \n");
		exit(0);  
	}
	//printf("Memory attached at %X\n", (int)share_mem);  

    //share_mem->sm_eth_setting = eth_setting;
}

void main(void)
{
    char strRTMPType[10];
    char strRTSPType[10];
    char strWiFiDHCP[10];
    char strWiFiType[10];
    char strWiFiEnable[10];
    char *data;
    char strTemp[10];
    char strControl[10];
    FILE *fp;
	
    printf("Content-Type:text/html\n\n");
    
    data = getenv("QUERY_STRING");
    
    if(NULL==data)
    {
        printf("Parameter Error \n");
        exit(0);
    }
    
    InitShareMem();
    
	//sscanf(data,"rtmp_type=%[^&]&rtsp_type=%[^&]&ip=%[^&]&mask=%[^&]&gateway=%[^&]&wifi_dhcp=%[^&]&wifi_ip=%[^&]&wifi_mask=%[^&]&wifi_gateway=%[^&]&wifi_type=%[^&]&wifi_essid=%[^&]&wifi_psk=%[^&]&wifi_enable=%[^&]",strRTMPType,strRTSPType,share_mem->sm_eth_setting.strEthIp,share_mem->sm_eth_setting.strEthMask,share_mem->sm_eth_setting.strEthGateway,strWiFiDHCP,share_mem->sm_wlan_setting.strWlanIp,share_mem->sm_wlan_setting.strWlanMask,share_mem->sm_wlan_setting.strWlanGateway,strWiFiType,share_mem->sm_wlan_setting.strWlanSSID,share_mem->sm_wlan_setting.strWlanPSK,strWiFiEnable);
    //sscanf(data,"rtmp_type=%[^&]&rtsp_type=%[^&]&ip=%[^&]&mask=%[^&]&gateway=%[^&]&wifi_dhcp=%[^&]&wifi_ip=%[^&]&wifi_mask=%[^&]&wifi_gateway=%[^&]&wifi_type=%[^&]&wifi_essid=%[^&]&wifi_psk=%[^&]&wifi_enable=%[^&]",strRTMPType,strRTSPType,share_mem->sm_eth_setting.strEthIp,share_mem->sm_eth_setting.strEthMask,share_mem->sm_eth_setting.strEthGateway,strWiFiDHCP,strTemp,strTemp,strTemp,strWiFiType,share_mem->sm_wlan_setting.strWlanSSID,share_mem->sm_wlan_setting.strWlanPSK,strWiFiEnable);
    //sscanf(data,"controlBox=%[^&]&rtmp_type=%[^&]&rtsp_type=%[^&]&ip=%[^&]&mask=%[^&]&gateway=%[^&]&multicast=%[^&]&wifi_dhcp=%[^&]&wifi_ip=%[^&]&wifi_mask=%[^&]&wifi_gateway=%[^&]&wifi_type=%[^&]&wifi_essid=%[^&]&wifi_psk=%[^&]&wifi_enable=%[^&]",strRTMPType,strRTSPType,share_mem->sm_eth_setting.strEthIp,share_mem->sm_eth_setting.strEthMask,share_mem->sm_eth_setting.strEthGateway,share_mem->sm_eth_setting.strEthMulticast,strWiFiDHCP,strTemp,strTemp,strTemp,strWiFiType,share_mem->sm_wlan_setting.strWlanSSID,share_mem->sm_wlan_setting.strWlanPSK,strWiFiEnable);
	sscanf(data,"controlBox=%[^&]&ip=%[^&]&mask=%[^&]&gateway=%[^&]&multicast=%[^&]",strControl,share_mem->sm_eth_setting.strEthIp,share_mem->sm_eth_setting.strEthMask,share_mem->sm_eth_setting.strEthGateway,share_mem->sm_eth_setting.strEthMulticast);
    share_mem->ucControlBoxFlag = atoi(strControl);
    //printf("%d", share_mem->ucControlBoxFlag);
    
    while(1==share_mem->ucUpdateFlag)
    {
        usleep(1000);
    }

	share_mem->ucUpdateFlag = 1;
	usleep(1000);
	printf("succeed");
}

