#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Setting.h"
#include "sharemem.h"

#define CMD1	"/bin/cp /tmp/configs/config.conf /tmp/configs/config1.conf"
#define CMD2	"/bin/cp /tmp/configs/config.conf /tmp/configs/config2.conf"
#define CMD3	"/bin/cp /tmp/configs/config.conf /tmp/configs/config3.conf"
#define CMD4	"/bin/cp /tmp/configs/config.conf /tmp/configs/config4.conf"
#define CMD5	"/bin/cp /tmp/configs/config.conf /tmp/configs/config5.conf"
#define CMD6	"/bin/cp /tmp/configs/config.conf /tmp/configs/config6.conf"
#define CMD7	"/bin/cp /tmp/configs/config.conf /tmp/configs/config7.conf"
#define CMD8	"/bin/cp /tmp/configs/config.conf /tmp/configs/config8.conf"
#define CMD9	"/bin/cp /tmp/configs/config.conf /tmp/configs/config9.conf"
#define CMD10	"/bin/cp /tmp/configs/config.conf /tmp/configs/config10.conf"

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
    //char strTemp[25] = {0};
	char strNum[10] = {0};
	char num;
    
	InitShareMem();
	printf("Content-Type:text/html\n\n");
    data = getenv("QUERY_STRING");
    
    if(NULL==data)
    {
        printf("Parameter Error \n");
        exit(0);
    }
    
    sscanf(data,"%*[^:]:%[^&]", strNum);
    
    //printf("num: %s str: %s \n", strNum, strTemp);
    num = atoi(strNum);
    	
	switch (num)
    {
		case 1:
			system(CMD1);
			break;	
		case 2:
			system(CMD2);
			break;
		case 3:
			system(CMD3);
			break;
		case 4:
			system(CMD4);
			break;
		case 5:
			system(CMD5);
			break;
		case 6:
			system(CMD6);
			break;	
		case 7:
			system(CMD7);
			break;
		case 8:
			system(CMD8);
			break;
		case 9:
			system(CMD9);
			break;
		case 10:
			system(CMD10);
			break;
		default:
			printf("error");
			break;
	}
	
	usleep(1000);
	share_mem->ucModeApplyFlag = 1;	
	printf("succeed");
}

