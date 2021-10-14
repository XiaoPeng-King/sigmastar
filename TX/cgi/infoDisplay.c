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

}

void main(void)
{
	char *data;
	char num;
	char strNum[10] = {0};
	
	printf("content-Type:text/html\n\n");
	InitShareMem();
	data = getenv("QUERY_STRING");
    
    if(NULL==data)
    {
        printf("Parameter Error \n");
        exit(0);
    }
    
    sscanf(data,"%*[^:]:%[^&]", strNum);
	num = atoi(strNum);
	if (1 == num)
	{
		share_mem->ucInfoDisplayFlag = 1;
	}
	else
	{
		share_mem->ucInfoDisplayFlag = 0;
	}
	usleep(10000);
	
	printf("succeed");
}

