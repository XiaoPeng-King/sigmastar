#include <stdio.h>
#include <stdlib.h>
 
#define CMD1	"/bin/cp /tmp/configs/config.conf /tmp/configs/config1.conf"
#define CMD2	"/bin/cp /tmp/configs/config.conf /tmp/configs/config2.conf"
#define CMD3	"/bin/cp /tmp/configs/config.conf /tmp/configs/config3.conf"
#define CMD4	"/bin/cp /tmp/configs/config.conf /tmp/configs/config4.conf"
#define CMD5	"/bin/cp /tmp/configs/config.conf /tmp/configs/config5.conf"

int main(void)
{
	char *data = NULL;
	char strNum[10] = {0};
	//char strNum;
	char num = -1;
	//FILE * fp = 0;
	//char strTemp[10] = {0};
	
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
    
    printf("succeed");
    //InitShareMem();
#endif

	return 0;
}
