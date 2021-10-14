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
    char strFrameRate[10];
    char strInterval[10];
    char strBitRate[10];

    char *data;

    printf("content-Type:text/html\n\n");
    
    data = getenv("QUERY_STRING");
    
    if(NULL==data)
    {
        printf("Parameter Error \n");
        exit(0);
    }   
    
    InitShareMem();
 
    sscanf(data,"BitRate=%[^&]&fps=%[^&]&gop=%[^&]&",strBitRate,strInterval,strFrameRate);
    
    share_mem->sm_encoder_setting.usEncRate= atoi(strBitRate);
    share_mem->sm_encoder_setting.ucFrameRate= atoi(strFrameRate);
    share_mem->sm_encoder_setting.ucInterval= atoi(strInterval);
        
    while(1==share_mem->ucUpdateFlag)
    {
        usleep(1000);
    }

	share_mem->ucUpdateFlag = 1;
	sleep(1);
	printf("succeed");
}

