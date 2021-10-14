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
    char *data;
    FILE *fp;
	char * type;
    char * method;
	
	char strTemp[25] = {0};
	char strNum[10] = {0};
	char num;
	
    printf("Content-Type:text/html\n\n");
    
    type = getenv("CONTENT_TYPE");
    //printf("<p>type : %s</p>",type);

    method = getenv("REQUEST_METHOD");
    //printf("<p>method : %s</p>", method);

    data = get_cgi_data(stdin, method);
    //printf("<p>input : %s</p>", data);
    
    //data = getenv("QUERY_STRING");
    
    if(NULL==data)
    {
        printf("Parameter Error \n");
        exit(0);
    }
    
    InitShareMem();
    
#if 0
    sscanf(data,"rx1Name=%[^&]&rx2Name=%[^&]&rx3Name=%[^&]&rx4Name=%[^&]&rx5Name=%[^&]&rx6Name=%[^&]&rx7Name=%[^&]&rx8Name=%[^&]&rx9Name=%[^&]&rx10Name=%[^&]&rx11Name=%[^&]&rx12Name=%[^&]&rx13Name=%[^&]&rx14Name=%[^&]&rx15Name=%[^&]&rx16Name=%[^&]&rx17Name=%[^&]&rx18Name=%[^&]&rx19Name=%[^&]&rx20Name=%[^&]&rx21Name=%[^&]&rx22Name=%[^&]&rx23Name=%[^&]&rx24Name=%[^&]\
				&rx25Name=%[^&]&rx26Name=%[^&]&rx27Name=%[^&]&rx28Name=%[^&]&rx29Name=%[^&]&rx30Name=%[^&]&rx31Name=%[^&]&rx32Name=%[^&]&rx33Name=%[^&]&rx34Name=%[^&]&rx35Name=%[^&]&rx36Name=%[^&]&rx37Name=%[^&]&rx38Name=%[^&]&rx39Name=%[^&]&rx40Name=%[^&]&rx41Name=%[^&]&rx42Name=%[^&]&rx43Name=%[^&]&rx44Name=%[^&]&rx45Name=%[^&]&rx46Name=%[^&]&rx47Name=%[^&]&rx48Name=%[^&]\
				&rx49Name=%[^&]&rx50Name=%[^&]&rx51Name=%[^&]&rx52Name=%[^&]&rx53Name=%[^&]&rx54Name=%[^&]&rx55Name=%[^&]&rx56Name=%[^&]&rx57Name=%[^&]&rx58Name=%[^&]&rx59Name=%[^&]&rx60Name=%[^&]&rx61Name=%[^&]&rx62Name=%[^&]&rx63Name=%[^&]&rx64Name=%[^&]&rx65Name=%[^&]&rx66Name=%[^&]&rx67Name=%[^&]&rx68Name=%[^&]&rx69Name=%[^&]&rx70Name=%[^&]&rx71Name=%[^&]&rx72Name=%[^&]\
				&rx73Name=%[^&]&rx74Name=%[^&]&rx75Name=%[^&]&rx76Name=%[^&]&rx77Name=%[^&]&rx78Name=%[^&]&rx79Name=%[^&]&rx80Name=%[^&]&rx81Name=%[^&]&rx82Name=%[^&]&rx83Name=%[^&]&rx84Name=%[^&]&rx85Name=%[^&]&rx86Name=%[^&]&rx87Name=%[^&]&rx88Name=%[^&]&rx89Name=%[^&]&rx90Name=%[^&]&rx91Name=%[^&]&rx92Name=%[^&]&rx93Name=%[^&]&rx94Name=%[^&]&rx95Name=%[^&]&rx96Name=%[^&]\
				&rx97Name=%[^&]&rx98Name=%[^&]&rx99Name=%[^&]&rx100Name=%[^&]&rx101Name=%[^&]&rx102Name=%[^&]&rx103Name=%[^&]&rx104Name=%[^&]&rx105Name=%[^&]&rx106Name=%[^&]&rx107Name=%[^&]&rx108Name=%[^&]&rx109Name=%[^&]&rx110Name=%[^&]&rx111Name=%[^&]&rx112Name=%[^&]&rx113Name=%[^&]&rx114Name=%[^&]&rx115Name=%[^&]&rx116Name=%[^&]&rx117Name=%[^&]&rx118Name=%[^&]&rx119Name=%[^&]&rx120Name=%[^&]\
				&rx121Name=%[^&]&rx122Name=%[^&]&rx123Name=%[^&]&rx124Name=%[^&]&rx125Name=%[^&]&rx126Name=%[^&]&rx127Name=%[^&]&rx128Name=%[^&]"

				,share_mem->sm_group_rename.rxRename[0],share_mem->sm_group_rename.rxRename[1],share_mem->sm_group_rename.rxRename[2],share_mem->sm_group_rename.rxRename[3],share_mem->sm_group_rename.rxRename[4],share_mem->sm_group_rename.rxRename[5],share_mem->sm_group_rename.rxRename[6],share_mem->sm_group_rename.rxRename[7],share_mem->sm_group_rename.rxRename[8],share_mem->sm_group_rename.rxRename[9],share_mem->sm_group_rename.rxRename[10],share_mem->sm_group_rename.rxRename[11],share_mem->sm_group_rename.rxRename[12],share_mem->sm_group_rename.rxRename[13],share_mem->sm_group_rename.rxRename[14],share_mem->sm_group_rename.rxRename[15],share_mem->sm_group_rename.rxRename[16],share_mem->sm_group_rename.rxRename[17],share_mem->sm_group_rename.rxRename[18],share_mem->sm_group_rename.rxRename[19],share_mem->sm_group_rename.rxRename[20],share_mem->sm_group_rename.rxRename[21],share_mem->sm_group_rename.rxRename[22],share_mem->sm_group_rename.rxRename[23]\
				,share_mem->sm_group_rename.rxRename[24],share_mem->sm_group_rename.rxRename[25],share_mem->sm_group_rename.rxRename[26],share_mem->sm_group_rename.rxRename[27],share_mem->sm_group_rename.rxRename[28],share_mem->sm_group_rename.rxRename[29],share_mem->sm_group_rename.rxRename[30],share_mem->sm_group_rename.rxRename[31],share_mem->sm_group_rename.rxRename[32],share_mem->sm_group_rename.rxRename[33],share_mem->sm_group_rename.rxRename[34],share_mem->sm_group_rename.rxRename[35],share_mem->sm_group_rename.rxRename[36],share_mem->sm_group_rename.rxRename[37],share_mem->sm_group_rename.rxRename[38],share_mem->sm_group_rename.rxRename[39],share_mem->sm_group_rename.rxRename[40],share_mem->sm_group_rename.rxRename[41],share_mem->sm_group_rename.rxRename[42],share_mem->sm_group_rename.rxRename[43],share_mem->sm_group_rename.rxRename[44],share_mem->sm_group_rename.rxRename[45],share_mem->sm_group_rename.rxRename[46],share_mem->sm_group_rename.rxRename[47]\
				,share_mem->sm_group_rename.rxRename[48],share_mem->sm_group_rename.rxRename[49],share_mem->sm_group_rename.rxRename[50],share_mem->sm_group_rename.rxRename[51],share_mem->sm_group_rename.rxRename[52],share_mem->sm_group_rename.rxRename[53],share_mem->sm_group_rename.rxRename[54],share_mem->sm_group_rename.rxRename[55],share_mem->sm_group_rename.rxRename[56],share_mem->sm_group_rename.rxRename[57],share_mem->sm_group_rename.rxRename[58],share_mem->sm_group_rename.rxRename[59],share_mem->sm_group_rename.rxRename[60],share_mem->sm_group_rename.rxRename[61],share_mem->sm_group_rename.rxRename[62],share_mem->sm_group_rename.rxRename[63],share_mem->sm_group_rename.rxRename[64],share_mem->sm_group_rename.rxRename[65],share_mem->sm_group_rename.rxRename[66],share_mem->sm_group_rename.rxRename[67],share_mem->sm_group_rename.rxRename[68],share_mem->sm_group_rename.rxRename[69],share_mem->sm_group_rename.rxRename[70],share_mem->sm_group_rename.rxRename[71]\
				,share_mem->sm_group_rename.rxRename[72],share_mem->sm_group_rename.rxRename[73],share_mem->sm_group_rename.rxRename[74],share_mem->sm_group_rename.rxRename[75],share_mem->sm_group_rename.rxRename[76],share_mem->sm_group_rename.rxRename[77],share_mem->sm_group_rename.rxRename[78],share_mem->sm_group_rename.rxRename[79],share_mem->sm_group_rename.rxRename[80],share_mem->sm_group_rename.rxRename[81],share_mem->sm_group_rename.rxRename[82],share_mem->sm_group_rename.rxRename[83],share_mem->sm_group_rename.rxRename[84],share_mem->sm_group_rename.rxRename[85],share_mem->sm_group_rename.rxRename[86],share_mem->sm_group_rename.rxRename[87],share_mem->sm_group_rename.rxRename[88],share_mem->sm_group_rename.rxRename[89],share_mem->sm_group_rename.rxRename[90],share_mem->sm_group_rename.rxRename[91],share_mem->sm_group_rename.rxRename[92],share_mem->sm_group_rename.rxRename[93],share_mem->sm_group_rename.rxRename[94],share_mem->sm_group_rename.rxRename[95]\
				,share_mem->sm_group_rename.rxRename[96],share_mem->sm_group_rename.rxRename[97],share_mem->sm_group_rename.rxRename[98],share_mem->sm_group_rename.rxRename[99],share_mem->sm_group_rename.rxRename[100],share_mem->sm_group_rename.rxRename[101],share_mem->sm_group_rename.rxRename[102],share_mem->sm_group_rename.rxRename[103],share_mem->sm_group_rename.rxRename[104],share_mem->sm_group_rename.rxRename[105],share_mem->sm_group_rename.rxRename[106],share_mem->sm_group_rename.rxRename[107],share_mem->sm_group_rename.rxRename[108],share_mem->sm_group_rename.rxRename[109],share_mem->sm_group_rename.rxRename[110],share_mem->sm_group_rename.rxRename[111],share_mem->sm_group_rename.rxRename[112],share_mem->sm_group_rename.rxRename[113],share_mem->sm_group_rename.rxRename[114],share_mem->sm_group_rename.rxRename[115],share_mem->sm_group_rename.rxRename[116],share_mem->sm_group_rename.rxRename[117],share_mem->sm_group_rename.rxRename[118],share_mem->sm_group_rename.rxRename[119]\
				,share_mem->sm_group_rename.rxRename[120],share_mem->sm_group_rename.rxRename[121],share_mem->sm_group_rename.rxRename[122],share_mem->sm_group_rename.rxRename[123],share_mem->sm_group_rename.rxRename[124],share_mem->sm_group_rename.rxRename[125],share_mem->sm_group_rename.rxRename[126],share_mem->sm_group_rename.rxRename[127]);
#endif
#if 1
	sscanf(data,"%*[^x]x%[^N]", strNum);
    sscanf(data,"%*[^=]=%[^&]", strTemp);
    
    //printf("num: %s str: %s \n", strNum, strTemp);
    num = atoi(strNum)-1;
    
    //sscanf(data,"tx1Name=%[^&]&tx2Name=%[^&]&tx3Name=%[^&]&tx4Name=%[^&]&tx5Name=%[^&]&tx6Name=%[^&]&tx7Name=%[^&]&tx8Name=%[^&]&tx9Name=%[^&]&tx10Name=%[^&]&tx11Name=%[^&]&tx12Name=%[^&]&tx13Name=%[^&]&tx14Name=%[^&]&tx15Name=%[^&]&tx16Name=%[^&]&tx17Name=%[^&]&tx18Name=%[^&]&tx19Name=%[^&]&tx20Name=%[^&]&tx21Name=%[^&]&tx22Name=%[^&]&tx23Name=%[^&]&tx24Name=%[^&]",share_mem->sm_group_rename.txRename[0],share_mem->sm_group_rename.txRename[1],share_mem->sm_group_rename.txRename[2],share_mem->sm_group_rename.txRename[3],share_mem->sm_group_rename.txRename[4],share_mem->sm_group_rename.txRename[5],share_mem->sm_group_rename.txRename[6],share_mem->sm_group_rename.txRename[7],share_mem->sm_group_rename.txRename[8],share_mem->sm_group_rename.txRename[9],share_mem->sm_group_rename.txRename[10],share_mem->sm_group_rename.txRename[11],share_mem->sm_group_rename.txRename[12],share_mem->sm_group_rename.txRename[13],share_mem->sm_group_rename.txRename[14],share_mem->sm_group_rename.txRename[15],share_mem->sm_group_rename.txRename[16],share_mem->sm_group_rename.txRename[17],share_mem->sm_group_rename.txRename[18],share_mem->sm_group_rename.txRename[19],share_mem->sm_group_rename.txRename[20],share_mem->sm_group_rename.txRename[21],share_mem->sm_group_rename.txRename[22],share_mem->sm_group_rename.txRename[23],share_mem->sm_group_rename.txRename[23]);
	
	strcpy(share_mem->sm_group_rename.rxRename[num], strTemp);

#endif

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

