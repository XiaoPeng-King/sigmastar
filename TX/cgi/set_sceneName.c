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

char* get_cgi_data(FILE* fp, char* method)
{
    char* input;
    int len;
    int size=1024*10;
    int i=0;

    if (strcmp(method, "GET") == 0)  /**< GET method */
    {
        input = getenv("QUERY_STRING");
        return input;
    }
    else if (strcmp(method, "POST") == 0)  /**< POST method */
    {
        len = atoi(getenv("CONTENT_LENGTH"));
        //printf("size:-%d-\n",len);
        input = (char*)malloc(sizeof(char) * (size+1));

        if (len == 0)
        {
            input[0] = '\0';
            return input;
        }

        while (1)
        {
            input[i] = (char)fgetc(fp);
            //printf("-%c-\n",input[i]);
            if (i == size)
            {
                //printf("1");
                input[i+1] = '\0';
                return input;
            }
            --len;

            if (feof(fp) || (!(len)))
            {
                i++;
                //printf("2");
                input[i] = '\0';
                return input;
            }
            i++;
        }
    }
    return NULL;
}

void main(void)
{
    char str[256];
    char strTemp[20];
    unsigned char i;
    
    //printf("init share memary \n");
    InitShareMem();
    
    strcat(str,"<set>");
    
#if 1
    for (i=0; i<5; i++)
    {
		sprintf(strTemp,"<Scene%d>", (i+1));
		strcat(str, strTemp);
		sprintf(strTemp,"%s", share_mem->sm_scene_rename.sceneRename[i]);
		strcat(str, strTemp);
		sprintf(strTemp, "</Scene%d>", (i+1));
		strcat(str, strTemp);
	}
#endif
    strcat(str,"</set>");
    
	printf("content-Type:text/xml\n\n");
	printf("%s\n",str);
	
}

