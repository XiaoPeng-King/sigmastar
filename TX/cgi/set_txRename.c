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
    unsigned char i;
    
    //printf("init share memary \n");
    InitShareMem();

    strcpy(str,"<set>");
    
    for (i=0; i<24; i++)
    {
		sprintf(strTemp,"<TX%d>", (i+1));
		strcat(str, strTemp);
		sprintf(strTemp,"%s", share_mem->sm_group_rename.txRename[i]);
		strcat(str, strTemp);
		sprintf(strTemp, "</TX%d>", (i+1));
		strcat(str, strTemp);
	}
    
    strcat(str,"</set>");
    
	printf("content-Type:text/xml\n\n");
	printf("%s\n",str);
	
}

