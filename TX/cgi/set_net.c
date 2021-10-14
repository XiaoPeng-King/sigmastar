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
    sscanf(data,"rtmp_type=%[^&]&rtsp_type=%[^&]&ip=%[^&]&mask=%[^&]&gateway=%[^&]&multicast=%[^&]&wifi_dhcp=%[^&]&wifi_ip=%[^&]&wifi_mask=%[^&]&wifi_gateway=%[^&]&wifi_type=%[^&]&wifi_essid=%[^&]&wifi_psk=%[^&]&wifi_enable=%[^&]",strRTMPType,strRTSPType,share_mem->sm_eth_setting.strEthIp,share_mem->sm_eth_setting.strEthMask,share_mem->sm_eth_setting.strEthGateway,share_mem->sm_eth_setting.strEthMulticast,strWiFiDHCP,strTemp,strTemp,strTemp,strWiFiType,share_mem->sm_wlan_setting.strWlanSSID,share_mem->sm_wlan_setting.strWlanPSK,strWiFiEnable);
	
#if 0
    share_mem->sm_rtmp_setting.ucRTMPInterface= atoi(strRTMPType);
    share_mem->sm_rtsp_setting.ucRTSPInterface= atoi(strRTSPType);
    share_mem->sm_wlan_setting.ucWlanDHCPSwitch= atoi(strWiFiDHCP);
    share_mem->sm_wlan_setting.ucWlanEyp=atoi(strWiFiType);
    share_mem->sm_wlan_setting.ucWlanEnable=atoi(strWiFiEnable);
    
    fp = fopen("/user/wpa_supplicant.conf","w");
    fprintf(fp,"ctrl_interface=wlan0\n");
    fprintf(fp,"network={\n");
    #if 1
    fprintf(fp,"    ssid=\"%s\"\n",share_mem->sm_wlan_setting.strWlanSSID);
    fprintf(fp,"    psk=\"%s\"\n",share_mem->sm_wlan_setting.strWlanPSK);
    #endif
    fprintf(fp,"}\n");
    fclose(fp);
#endif
	#if 0
    if(1==share_mem->sm_wlan_setting.ucWlanEyp)
    {
        fprintf(fp,"    ssid=\"%s\"\n",share_mem->sm_wlan_setting.strWlanSSID);
        fprintf(fp,"    psk=\"%s\"\n",share_mem->sm_wlan_setting.strWlanPSK);
    }
    else if(0 == share_mem->sm_wlan_setting.ucWlanEyp)
    {
        fprintf(fp,"    ssid=\"%s\"\n",share_mem->sm_wlan_setting.strWlanSSID);
        fprintf(fp,"    key_mgmt=NONE");    
    }
    else if(2 == share_mem->sm_wlan_setting.ucWlanEyp)
    {
        fprintf(fp,"    ssid=\"%s\"\n",share_mem->sm_wlan_setting.strWlanSSID);
        fprintf(fp,"    key_mgmt=NONE");    
        fprintf(fp,"    wep_key0=\"%s\"\n",share_mem->sm_wlan_setting.strWlanPSK);        
    }
    #endif
   
    
    while(1==share_mem->ucUpdateFlag)
    {
        usleep(1000);
    }

	share_mem->ucUpdateFlag = 1;
	usleep(1000);
	printf("succeed");
}

