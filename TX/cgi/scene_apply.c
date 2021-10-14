#include <stdio.h>
#include <stdlib.h>
#include "sharemem.h"
 
#define CMD1	"/bin/cp /tmp/configs/config1.conf /tmp/configs/config.conf"
#define CMD2	"/bin/cp /tmp/configs/config2.conf /tmp/configs/config.conf"
#define CMD3	"/bin/cp /tmp/configs/config3.conf /tmp/configs/config.conf"
#define CMD4	"/bin/cp /tmp/configs/config4.conf /tmp/configs/config.conf"
#define CMD5	"/bin/cp /tmp/configs/config5.conf /tmp/configs/config.conf"

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


int main(void)
{
	char *data = NULL;
	char strNum[10] = {0};
	//char strNum;
	char num = -1;
	//FILE * fp = 0;
	//char strTemp[10] = {0};
	
	InitShareMem();
	
	printf("Content-Type:text/html\n\n");
#if 1
    data = getenv("QUERY_STRING");
    
    if(NULL==data)
    {
        printf("Parameter Error \n");
        exit(0);
    }
    
    sscanf(data,"%*[^=]=%[^&]", strNum);
    num = atoi(strNum);

    //printf("%d", num);
    
    //printf("%c", strNum);
    switch (num)
    {
		case 0:
			/*
			fp = popen(CMD1, "r");
			if(fp == NULL){
				printf("set scene file...\n");
				pclose(fp);
				return -1;
			}
			pclose(fp);
			*/
			system(CMD1);
			break;	
		case 1:
			//printf("1");
			system(CMD2);
			break;
		case 2:
			system(CMD3);
			break;
		case 3:
			system(CMD4);
			break;
		case 4:
			system(CMD5);
			break;
		default:
			printf("error");
			break;
	}
	while(1==share_mem->ucModeApplyFlag)
    {
        usleep(1000);
    }
	share_mem->ucModeApplyFlag = 1;
	usleep(1000);
    
    printf("succeed");
    //InitShareMem();
#endif

	return 0;
}
