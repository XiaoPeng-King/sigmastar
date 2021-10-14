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
//	printf("Memory attached at %X\n", (int)share_mem);  

    //share_mem->sm_eth_setting = eth_setting;
}

void main(void)
{
    char str[2048];

    
    InitShareMem();

    strcpy(str,"<sys>");
    strcat(str,"<hwv>");
    strcat(str,share_mem->sm_run_status.strHardwareVer);
    strcat(str,"</hwv>");

    strcat(str,"<swv>");
    strcat(str,share_mem->sm_run_status.strSoftwareVer);
    strcat(str,"</swv>");

    strcat(str,"<cec_off_code>");
    strcat(str,share_mem->cec_control.cec_code_off);
    strcat(str,"</cec_off_code>");

    strcat(str,"<cec_on_code>");
    strcat(str,share_mem->cec_control.cec_code_on);
    strcat(str,"</cec_on_code>");

    strcat(str,"</sys>");
    
     printf("content-Type:text/xml\n\n");
       printf("%s\n",str);
       
   
}

