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
		printf("shmget failed : %d\n", shmid);  
		return 0;//exit(0);  
	}

	share_mem = shmat(shmid, (void*)0, 0);  
	if(share_mem == NULL)  
	{  
		printf("shmat failed\n");	
		return 0;
		//exit(0);  
	}
}

unsigned char ASCII_To_Hex(unsigned char number)
{
 
	if (number >= '0' && number <= '9')
		return (number - 0x30);
 
	else if (number >= 'a' && number <= 'f')
		return ((number - 'a') + 10);
	
	else if (number >= 'A' && number <= 'F')
		return ((number - 'A') + 10);
 
	return (0);
}

void main(void)
{
    char *data;
	char strCode[20] = {0};
	char Hex1 = 0, Hex2 = 0;
    
	printf("Content-Type:text/html\n\n");
    
    data = getenv("QUERY_STRING");
    
    if(NULL==data)
    {
        printf("Parameter Error \n");
        exit(0);
    }
	InitShareMem();
    
	sscanf(data,"cec_on_code=%[^&]&", strCode);
	//printf("%s", data);
	if (((('0' <= strCode[0]) && (strCode[0] <= '9')) || (('a' <=strCode[0]) && (strCode[0] <='f'))) && (strCode[2] == '-'))
    {
            strcpy(share_mem->cec_control.cec_code_on, strCode);
            share_mem->cec_control.cec_state = 1; //on
    }
    else if (0 == strCode[0] && 0 ==strCode[1])
    {
            strcpy(share_mem->cec_control.cec_code_on,"40-04");
            share_mem->cec_control.cec_state = 1; //on
    }
    else
    {

            share_mem->cec_control.cec_state = 0; //init
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
