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
	char strCode[20] = {0};
	
	printf("Content-Type:text/html\n\n");
    
    data = getenv("QUERY_STRING");
    
    if(NULL==data)
    {
        printf("Parameter Error \n");
        exit(0);
    }
	InitShareMem();

    sscanf(data,"cec_off_code=%[^&]&", strCode);
	//printf("strCode : %s", strCode);
	if (((('0' <= strCode[0]) && (strCode[0] <= '9')) || (('a' <=strCode[0]) && (strCode[0] <='f'))) && (strCode[2] == '-'))
    {
            strcpy(share_mem->cec_control.cec_code_off, strCode);
            share_mem->cec_control.cec_state = 2; //on
    }
    else if (0 == strCode[0] && 0 ==strCode[1])
    {
            strcpy(share_mem->cec_control.cec_code_off,"ff-36");
            share_mem->cec_control.cec_state = 2; //on
    }
    else
    {

            share_mem->cec_control.cec_state = 0; // init 
            printf("failed ,please input again !");
    }
#if 1
    while(1==share_mem->ucUpdateFlag)
    {
        usleep(1000);
    }
#endif
	share_mem->ucUpdateFlag = 1;
	usleep(1000);
	
	printf("succeed");
}
