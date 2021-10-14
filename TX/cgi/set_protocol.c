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
//	printf("Memory attached at %X\n", (int)share_mem);  

    //share_mem->sm_eth_setting = eth_setting;
}

void main(void)
{
    char strRTMPEnb[10];
    char strRTSPEnb[10];
    char strRTSPPort[10];
    char strRTPPort[10];
    char strRTMPPort[10];
    char *data;

    printf("content-Type:text/html\n\n");
    
    data = getenv("QUERY_STRING");
    
    if(NULL==data)
    {
        printf("Parameter Error \n");
        exit(0);
    }   
    
    InitShareMem();
   // sscanf(data,"rtmp_type=%[^&]&rtsp_type=%[^&]&ip=%[^&]&mask=%[^&]&gateway=%[^&]&wifi_dhcp=%[^&]&wifi_ip=%[^&]&wifi_mask=%[^&]&wifi_gateway=%[^&]&wifi_type=%[^&]&wifi_essid=%[^&]&wifi_psk=%[^&]",strRTMPType,strRTSPType,share_mem->sm_eth_setting.strEthIp,share_mem->sm_eth_setting.strEthMask,share_mem->sm_eth_setting.strEthGateway,strWiFiDHCP,share_mem->sm_wlan_setting.strWlanIp,share_mem->sm_wlan_setting.strWlanMask,share_mem->sm_wlan_setting.strWlanGateway,strWiFiType,share_mem->sm_wlan_setting.strWlanSSID,share_mem->sm_wlan_setting.strWlanPSK);
     sscanf(data,"rtsp_name=%[^&]&rtsp_sle=%[^&]&rtsp_port=%[^&]&mul_ip=%[^&]&mul_port=%[^&]&rtmp_port=%[^&]&rtmp_sle=%[^&]&rtmp_url=%[^&]&",share_mem->sm_rtsp_setting.strRTSPUrl,strRTSPEnb,strRTSPPort,share_mem->sm_rtsp_setting.strRTPBroadcastIp,strRTPPort,strRTMPPort,strRTMPEnb,share_mem->sm_rtmp_setting.strRTMPUrl);
    
    share_mem->sm_rtsp_setting.usRTSPPort= atoi(strRTSPPort);
    share_mem->sm_rtsp_setting.ucRTSPSwitch= atoi(strRTSPEnb);
    share_mem->sm_rtsp_setting.usRTPPort= atoi(strRTPPort);
    share_mem->sm_rtmp_setting.usRTMPPort= atoi(strRTMPPort);
    share_mem->sm_rtmp_setting.ucRTMPSwitch= atoi(strRTMPEnb);
    
    while(1==share_mem->ucUpdateFlag)
    {
        usleep(1000);
    }

        share_mem->ucUpdateFlag = 1;
        sleep(1);
        printf("succeed");
}

