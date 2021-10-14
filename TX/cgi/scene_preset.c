#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Setting.h"
#include "sharemem.h"

#define CMD1	"/bin/cp /tmp/configs/config6.conf /tmp/configs/config1.conf"
#define CMD2	"/bin/cp /tmp/configs/config6.conf /tmp/configs/config2.conf"
#define CMD3	"/bin/cp /tmp/configs/config6.conf /tmp/configs/config3.conf"
#define CMD4	"/bin/cp /tmp/configs/config6.conf /tmp/configs/config4.conf"
#define CMD5	"/bin/cp /tmp/configs/config6.conf /tmp/configs/config5.conf"

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
    char strTemp[25] = {0};
	char strNum[10] = {0};
	char num;
    
    FILE *fp;
    
	InitShareMem();
	
    printf("Content-Type:text/html\n\n");
    
    data = getenv("QUERY_STRING");
    
    if(NULL==data)
    {
        printf("Parameter Error \n");
        exit(0);
    }
    
    sscanf(data,"%*[^=]=%[^&]", strNum);
    //sscanf(data,"%*[^=]=%[^&]", strTemp);
    
    //printf("num: %s str: %s \n", strNum, strTemp);
    num = atoi(strNum);
    
    //sscanf(data,"tx1Name=%[^&]&tx2Name=%[^&]&tx3Name=%[^&]&tx4Name=%[^&]&tx5Name=%[^&]&tx6Name=%[^&]&tx7Name=%[^&]&tx8Name=%[^&]&tx9Name=%[^&]&tx10Name=%[^&]&tx11Name=%[^&]&tx12Name=%[^&]&tx13Name=%[^&]&tx14Name=%[^&]&tx15Name=%[^&]&tx16Name=%[^&]&tx17Name=%[^&]&tx18Name=%[^&]&tx19Name=%[^&]&tx20Name=%[^&]&tx21Name=%[^&]&tx22Name=%[^&]&tx23Name=%[^&]&tx24Name=%[^&]",share_mem->sm_group_rename.txRename[0],share_mem->sm_group_rename.txRename[1],share_mem->sm_group_rename.txRename[2],share_mem->sm_group_rename.txRename[3],share_mem->sm_group_rename.txRename[4],share_mem->sm_group_rename.txRename[5],share_mem->sm_group_rename.txRename[6],share_mem->sm_group_rename.txRename[7],share_mem->sm_group_rename.txRename[8],share_mem->sm_group_rename.txRename[9],share_mem->sm_group_rename.txRename[10],share_mem->sm_group_rename.txRename[11],share_mem->sm_group_rename.txRename[12],share_mem->sm_group_rename.txRename[13],share_mem->sm_group_rename.txRename[14],share_mem->sm_group_rename.txRename[15],share_mem->sm_group_rename.txRename[16],share_mem->sm_group_rename.txRename[17],share_mem->sm_group_rename.txRename[18],share_mem->sm_group_rename.txRename[19],share_mem->sm_group_rename.txRename[20],share_mem->sm_group_rename.txRename[21],share_mem->sm_group_rename.txRename[22],share_mem->sm_group_rename.txRename[23],share_mem->sm_group_rename.txRename[23]);
	switch (num)
    {
		case 0:
			strcpy(share_mem->sm_scene_rename.sceneRename[0], "Scene1");
			system(CMD1);
			break;
		case 1:
			strcpy(share_mem->sm_scene_rename.sceneRename[1], "Scene2");
			system(CMD2);
			break;
		case 2:
			strcpy(share_mem->sm_scene_rename.sceneRename[2], "Scene3");
			system(CMD3);
			break;
		case 3:
			strcpy(share_mem->sm_scene_rename.sceneRename[3], "Scene4");
			system(CMD4);
			break;
		case 4:
			strcpy(share_mem->sm_scene_rename.sceneRename[4], "Scene5");
			system(CMD5);
			break;
		default:
			printf("error");
			break;
	}

#if 1
    while(1==share_mem->ucSceneApplyFlag)
    {
        usleep(1000);
    }
#endif
	share_mem->ucUpdateFlag = 1;
	usleep(1000);
	printf("succeed");
}

