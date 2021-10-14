#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Setting.h"
#include "sharemem.h"

#define CMD1	"/bin/cp /tmp/configs/config1.conf /tmp/configs/config.conf"
#define CMD2	"/bin/cp /tmp/configs/config2.conf /tmp/configs/config.conf"
#define CMD3	"/bin/cp /tmp/configs/config3.conf /tmp/configs/config.conf"
#define CMD4	"/bin/cp /tmp/configs/config4.conf /tmp/configs/config.conf"
#define CMD5	"/bin/cp /tmp/configs/config5.conf /tmp/configs/config.conf"
#define CMD6	"/bin/cp /tmp/configs/config6.conf /tmp/configs/config.conf"
#define CMD7	"/bin/cp /tmp/configs/config7.conf /tmp/configs/config.conf"
#define CMD8	"/bin/cp /tmp/configs/config8.conf /tmp/configs/config.conf"
#define CMD9	"/bin/cp /tmp/configs/config9.conf /tmp/configs/config.conf"
#define CMD10	"/bin/cp /tmp/configs/config10.conf /tmp/configs/config.conf"

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
    
    //sscanf(data,"%*[^_]_%[^=]", strNum);
    sscanf(data,"%*[^:]:%[^&]", strNum);
    //sscanf(data,"%*[^=]=%[^&]", strTemp);
    
    //printf("num: %s str: %s \n", strNum, strTemp);
    num = atoi(strNum);
    
    //sscanf(data,"tx1Name=%[^&]&tx2Name=%[^&]&tx3Name=%[^&]&tx4Name=%[^&]&tx5Name=%[^&]&tx6Name=%[^&]&tx7Name=%[^&]&tx8Name=%[^&]&tx9Name=%[^&]&tx10Name=%[^&]&tx11Name=%[^&]&tx12Name=%[^&]&tx13Name=%[^&]&tx14Name=%[^&]&tx15Name=%[^&]&tx16Name=%[^&]&tx17Name=%[^&]&tx18Name=%[^&]&tx19Name=%[^&]&tx20Name=%[^&]&tx21Name=%[^&]&tx22Name=%[^&]&tx23Name=%[^&]&tx24Name=%[^&]",share_mem->sm_group_rename.txRename[0],share_mem->sm_group_rename.txRename[1],share_mem->sm_group_rename.txRename[2],share_mem->sm_group_rename.txRename[3],share_mem->sm_group_rename.txRename[4],share_mem->sm_group_rename.txRename[5],share_mem->sm_group_rename.txRename[6],share_mem->sm_group_rename.txRename[7],share_mem->sm_group_rename.txRename[8],share_mem->sm_group_rename.txRename[9],share_mem->sm_group_rename.txRename[10],share_mem->sm_group_rename.txRename[11],share_mem->sm_group_rename.txRename[12],share_mem->sm_group_rename.txRename[13],share_mem->sm_group_rename.txRename[14],share_mem->sm_group_rename.txRename[15],share_mem->sm_group_rename.txRename[16],share_mem->sm_group_rename.txRename[17],share_mem->sm_group_rename.txRename[18],share_mem->sm_group_rename.txRename[19],share_mem->sm_group_rename.txRename[20],share_mem->sm_group_rename.txRename[21],share_mem->sm_group_rename.txRename[22],share_mem->sm_group_rename.txRename[23],share_mem->sm_group_rename.txRename[23]);
	
	share_mem->ucCurrentMode = num;
	//printf("%d", num);
	switch (num)
    {
		case 1:
			system(CMD1);
			//system("/bin/cp /tmp/configs/config1.conf /tmp/configs/config.conf");
			//printf("CMD1");
			break;	
		case 2:
			system(CMD2);
			//printf("CMD2");
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
#if 1
    while(1==share_mem->ucModeApplyFlag)
    {
        usleep(1000);
    }
#endif
	share_mem->ucModeApplyFlag = 1;
	usleep(1000);
	
	printf("succeed");
}

