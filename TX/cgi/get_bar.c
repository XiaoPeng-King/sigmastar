#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Setting.h"
#include "sharemem.h"

int InitShareMemUpload(void)
{

	int shmid;	

	shmid = shmget((key_t)1235, sizeof(SHARE_UPLOAD), 0666|IPC_CREAT);	
	if(shmid == -1)  
	{  
		printf("shmget failed\n");  
		exit(0);  
	}  

	share_upload = shmat(shmid, (void*)0, 0);  
	if(share_upload == NULL)  
	{  
		printf("shmat failed\n");	
		exit(0);  
	}  
	//printf("Memory attached at %X\n", (int)share_upload);  

    //share_mem->sm_eth_setting = eth_setting;
}

void main(void)
{
    unsigned int uiProgress;
    
    InitShareMemUpload();
    printf("share_upload->uiWriteLen = %d \n",share_upload->uiWriteLen);
    printf("share_upload->uiFileLen = %d \n",share_upload->uiFileLen);    
    if(share_upload->uiFileLen > 0)
        uiProgress = (100 * share_upload->uiWriteLen) / share_upload->uiFileLen;
    else
        uiProgress = 0;
        
    if(uiProgress > 96)
        uiProgress = 100;
    //printf("100");        
     printf("content-Type:text/xml\n\n");

       printf("%d",uiProgress);
    //printf("100");
}

