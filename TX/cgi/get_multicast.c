#include <stdio.h>
#include <stdlib.h>
#include "Setting.h"
#include "sharemem.h"

int InitShareMem(void)
{

	int shmid;

	shmid = shmget((key_t)1234, sizeof(SHARE_MEM), 0666|IPC_CREAT);	
	if(shmid == -1)  
	{  
		//printf("shmget failed\n");  
        printf("fail \n");
		exit(0);
	}  

	share_mem = shmat(shmid, (void*)0, 0);  
	if(share_mem == NULL)  
	{ 
		//printf("shmat failed\n");	
        printf("fail \n");
		exit(0);  
	}
	//printf("Memory attached at %X\n", (int)share_mem);  

    //share_mem->sm_eth_setting = eth_setting;
}


void main(void)
{
    char *data;
    char strTemp[10] = {0};
	char strNum[10] = {0};
	char num; 
	char multicast;
	
    printf("Content-Type:text/html\n\n");
#if 1
    data = getenv("QUERY_STRING");
    
    if(NULL==data)
    {
        printf("Parameter Error \n");
        exit(0);
    }
    
    InitShareMem();
#endif
	//sscanf(data,"rtmp_type=%[^&]&rtsp_type=%[^&]&ip=%[^&]&mask=%[^&]&gateway=%[^&]&wifi_dhcp=%[^&]&wifi_ip=%[^&]&wifi_mask=%[^&]&wifi_gateway=%[^&]&wifi_type=%[^&]&wifi_essid=%[^&]&wifi_psk=%[^&]&wifi_enable=%[^&]",strRTMPType,strRTSPType,share_mem->sm_eth_setting.strEthIp,share_mem->sm_eth_setting.strEthMask,share_mem->sm_eth_setting.strEthGateway,strWiFiDHCP,share_mem->sm_wlan_setting.strWlanIp,share_mem->sm_wlan_setting.strWlanMask,share_mem->sm_wlan_setting.strWlanGateway,strWiFiType,share_mem->sm_wlan_setting.strWlanSSID,share_mem->sm_wlan_setting.strWlanPSK,strWiFiEnable);
    //sscanf(data,"rtmp_type=%[^&]&rtsp_type=%[^&]&ip=%[^&]&mask=%[^&]&gateway=%[^&]&wifi_dhcp=%[^&]&wifi_ip=%[^&]&wifi_mask=%[^&]&wifi_gateway=%[^&]&wifi_type=%[^&]&wifi_essid=%[^&]&wifi_psk=%[^&]&wifi_enable=%[^&]",strRTMPType,strRTSPType,share_mem->sm_eth_setting.strEthIp,share_mem->sm_eth_setting.strEthMask,share_mem->sm_eth_setting.strEthGateway,strWiFiDHCP,strTemp,strTemp,strTemp,strWiFiType,share_mem->sm_wlan_setting.strWlanSSID,share_mem->sm_wlan_setting.strWlanPSK,strWiFiEnable);
    //sscanf(data,"selectRX%[1-9]=%[^&]",strNum, strTemp);
    
    sscanf(data,"%*[^X]X%[^=]", strNum);
    sscanf(data,"%*[^=]=%[^&]", strTemp);
    
    //sscanf("selectRX060=01&","%[1-9]%[^&]",strNum, strTemp);
    //sscanf("selectRX160=01&","%*[^X]X%[^=]", strNum);
    //sscanf("selectRX160=01&","%*[^=]=%[^&]", strTemp);
    //sscanf("1111/1111@iiiii", "%*[^/]/%[^@]", strNum);

    //printf(strNum);
    //printf(strTemp);
    
    num = atoi(strNum);
    multicast = atoi(strTemp);
    
    //printf("num = %d \n", num);
    //printf("multicast = %d \n", multicast);
    if ((multicast < 25) && (num <= 128))
    {
		share_mem->sm_group_pack.ucMultiAddress[num-1] = multicast; 
	}
	else
	{
		printf("set failed");
	}
    while(1==share_mem->ucUpdateFlag)
    {
        usleep(1000);
    }

	share_mem->ucUpdateFlag = 1;
	usleep(1000);
	printf("succeed");
}


