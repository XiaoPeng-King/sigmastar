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
		printf("shmget failed\n");  
		exit(0);  
	}  

	share_mem = shmat(shmid, (void*)0, 0);  
	if(share_mem == NULL)  
	{  
		printf("shmat failed\n");	
		exit(0);  
	} 
	//printf("Memory attached at %X\n", (int)share_mem);  

    //share_mem->sm_eth_setting = eth_setting;
}

void main(void)
{
    char str[2048];
    char strTemp[100];
    char strTempC[100];
    
    InitShareMem();
    if(0==share_mem->sm_run_status.ucInputStatus)
    {
     printf("content-Type:text/xml\n\n");
       printf("<set><input_type> No Input</input_type><input_video> ---</input_video><input_audio> ---</input_audio><RTSPC>未启动</RTSPC><RTPC>未启动</RTPC><RTMPC>未启动</RTMPC><RTSPE>OFF</RTSPE><RTPE>OFF</RTPE><RTMPE>OFF</RTMPE></set>\n");
       return;
    
    }

    strcpy(str,"<set>");
    strcat(str,"<input_type>");
    if(1==share_mem->sm_run_status.ucInputStatus)
    {
        strcat(str,"HDMI");
    }
    if(2==share_mem->sm_run_status.ucInputStatus)
    {
        strcat(str,"SDI");
    }
    
    strcat(str,"</input_type>");
    
    strcat(str,"<input_video>");
    sprintf(strTemp,"%d * %d @ %dHz",share_mem->sm_run_status.usWidth, share_mem->sm_run_status.usHeight, share_mem->sm_run_status.ucFrameRate);
    strcat(str,strTemp);
    strcat(str,"</input_video>");
    
    strcat(str,"<input_audio>");
    sprintf(strTemp,"PCM %dHz",share_mem->sm_run_status.usAudioSampleRate);
    strcat(str,strTemp);
    strcat(str,"</input_audio>");

    strcat(str,"</set>");
    
    printf("content-Type:text/xml\n\n");
    printf("%s\n",str);
       
//   printf("<set><ip>192.168.100.30</ip><mask>255.255.255.0</mask><gateway>192.168.0.1</gateway><dns0>192.168.0.1</dns0><dns1>192.168.0.1</dns1><mac>16:20:05:00:10:62</mac><dhcp>0</dhcp><net_type>1</net_type><wifi_exist>1</wifi_exist><wifi_ip>192.168.0.32</wifi_ip><wifi_netmask>255.255.255.0</wifi_netmask><wifi_essid></wifi_essid><wifi_psk></wifi_psk><wifi_gateway>192.168.0.1</wifi_gateway><wifi_type>0</wifi_type><wifi_status>0</wifi_status></set>\n");
    
}

