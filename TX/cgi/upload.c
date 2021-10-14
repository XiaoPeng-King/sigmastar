#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "dirent.h"
#include <wchar.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sharemem.h"

int InitShareMemUpload(void)
{

	int shmid;	

	shmid = shmget((key_t)1235, sizeof(SHARE_UPLOAD), 0666|IPC_CREAT);	
	if(shmid == -1)  
	{  
		printf("shmget failed\n");  
		exit(0);  
	}  
    printf("shmid = %d\n",shmid);

	share_upload = (struct shared_use_st*)shmat(shmid, (void*)0, 0);  
	printf("share_mem = %d\n",share_mem);
	
	if(share_upload == NULL)  
	{  
		printf("shmat failed\n");	
		exit(0);  
	}  
	printf("Memory attached at %X\n", (int)share_upload);  

    //share_mem->sm_eth_setting = eth_setting;
}


static int atoii (char *zzzz)
{
  int i = 0;
  int num=0;
for(i=0;i<20;i++)
{
  if(zzzz[i] >= '0' && zzzz[i] <= '9')
  	{
	  num = num * 10 + (zzzz[i] - '0');
 	}else
 	{
 			break;
 	}
}
  return num;
}


char* getCgiData(FILE* fp, char* requestmethod)
{

       char* input;
       int len;
	   char *pppp;
       int size = 1024;
       int i = 0;
	if(!strcmp(requestmethod, "GET"))
       {

              input = getenv("QUERY_STRING");
              return input;

       }
       else if (!strcmp(requestmethod, "POST"))
       {
   		    pppp=getenv("CONTENT_LENGTH");
             len = atoii(pppp);
              input = (char*)malloc(sizeof(char)*(size + 1));    

              if (len == 0)
              {
                     input[0] = '\0';

                     return input;
              }
        
		fgets(input, len+1, stdin);
			input[len]='\0';
		 return input;
       }

       return NULL;
}

static unsigned int tmppp=0;

char *getFileName(unsigned char *req)
{
	int i;
	int leng;
	tmppp=0;
	char *psz1; char *psz2;
	unsigned char *cur_post,*buf;

	// get filename keyword
	if ((psz1=strstr(req, "filename=")) == NULL)
	{
	return (char *)&tmppp;
	}

	// get pointer to actual filename (it's in quotes)
	psz1+=strlen("filename=");
	if ((psz1 = strtok(psz1, "\"")) == NULL)
	{
	return (char *)&tmppp;
	}
	// remove leading path for both PC and UNIX systems
	if ((psz2 = strrchr(psz1,'\\')) != NULL)
	{
	psz1 = psz2+1;
	}
	if ((psz2 = strrchr(psz1,'/')) != NULL)
	{
	psz1 = psz2+1;
	}
	return psz1;
}


void main()
{
	char *reqMethod;
	char *wp;
	char *var=NULL;
	int len;
	long total,i,count;
	char *fileName,*ps1,*ps2;
	char *fileN;
	char Boundary[256];
	char errorBuf[200]="";
	char tmpBuf[512];
	char filePath[256]="./";//directory of uploaded file
	char strCmd[256]="rm ";
	FILE *fileBuf=NULL;
	
	InitShareMemUpload();
	share_upload->uiWriteLen = 0;	
		
	reqMethod=getenv("REQUEST_METHOD");
	len=atoii(getenv("CONTENT_LENGTH"));
	printf("%s","Content-type:text/html \r\n\r\n");
	//printf("len = %d",len);
	
	share_upload->uiFileLen = len;
	//printf("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=UTF-8\"></head>");

	Boundary[0] = '\r';
	Boundary[1] = '\n';
	Boundary[2] = '\0';

	if (fgets(&Boundary[2], sizeof(Boundary)-2, stdin) == NULL)
	{
		sprintf(errorBuf,"%s","Get boundary failed !");
		printf("get boundary failed \n");
		goto error;
	}

	//strip terminating CR / LF
	if ((ps1=strchr(&Boundary[2],'\r')) != NULL)
	{
		*ps1 = '\0';
	}

	if ((ps1=strchr(&Boundary[2],'\n')) != NULL)
	{
		*ps1 = '\0';
	}
	//printf("Boundry=\"%s\"<br>",Boundary);

	//printf("content-length=%d<br>",len);
	fgets(tmpBuf,512,stdin);
	//printf("All=%s<br>",tmpBuf);
	fileName=getFileName(tmpBuf);
/*
	if(fileName)
	{
		//printf("fileName=%s<br>",fileName);
	}
*/
    if(0==strcmp(fileName,"viuhs.img"))
    {
        strcpy(filePath,"/user/update/bin/");
        strcpy(fileName,"viu-vpu0h264enc-mdev-rtsp");
        system("/bin/killall viu-vpu0h264enc-mdev-rtsp");
       
    }
    else if(strstr(fileName,".html")>0)
    {
        //printf("filename=%s\n",fileName);
         strcpy(filePath,"/debug/html/");
    }
    else if(strstr(fileName,".cgi")>0)
    {
        //printf("2\n");
        strcpy(filePath,"/debug/html/cgi-bin/");
    }
    else if(strstr(fileName,".ko")>0)
    {
        //printf("2\n");
        strcpy(filePath,"/user/update/ko/");
    }
    else
    {
        printf("fail\n");
        return;
    
    }    	
	strcat(filePath,fileName);	
	//printf("filepath===%s<br>",filePath);

	memset(tmpBuf,512,0x00);
	fgets(tmpBuf,512,stdin);
	//printf("%s<br>",tmpBuf);//content-type
	memset(tmpBuf,512,0x00);
	fgets(tmpBuf,512,stdin);
	
	
//	strcat(strCmd,filePath);
//	system(strCmd);
	
	if(remove(filePath)!=0)
	{
	    printf("r error");
	//    goto error;
	    
	}	
	else
	{
//	    printf("r ok");
	}
	//printf("%s<br>",tmpBuf);// \r\n
/*
	if(fopen(filePath,"rb"))
	{
		sprintf(errorBuf,"%s","File already exist.");
		printf("file existed");
	 	goto error;
	}
*/
	if ((fileBuf = fopen(filePath, "wb+")) == NULL)
	{
		sprintf(errorBuf,"%s","File open error.Make sure you have the permission.");
		printf("file open error");
		goto error;
	}
	// copy the file
	while ((count=fread(tmpBuf, 1, 512, stdin)) != 0)
	{
		if ((fwrite(tmpBuf, 1, count, fileBuf)) != count)
		{
			sprintf(errorBuf,"%s","Write file error.");
			printf("write file error");
			goto error;
		}
			share_upload->uiWriteLen += count;
	}
	// re read last 128 bytes of file, handling files < 128 bytes
	if ((count = ftell(fileBuf)) == -1)
	{
		printf("ftell fail");
		goto error;
	}

	if (count > 128)
	{
		count = 128;
	}

	if (fseek(fileBuf, 0-count, SEEK_END) != 0)
	{
		printf("fseek error");
		goto error;
	}
	// get the new position
	if ((total = ftell(fileBuf)) == -1)
	{
		printf("ftell2 fail");
		goto error;
	}

	// and read the data from fileBuf
	count = fread(tmpBuf, 1, sizeof(tmpBuf), fileBuf);
	tmpBuf[count] = '\0';
	//printf("count=%ld<br>",count);
	// determine offset of terminating boundary line
	for (i=0; i<=(count); i++)//-(long)strlen(Boundary)
	{
		//printf("%c",tmpBuf[i]);
		if (tmpBuf[i] == Boundary[0])
		{
			//printf("found /r<br>");
			if(strncmp(Boundary, &tmpBuf[i], strlen(Boundary)) == 0)
			{
			total+=i;
		//	printf("find boudary.<br>");
			break;
			}
		}
	}
	//printf("<br>i=%ld<br>",i);
	//printf("total=%ld<br>",total);
	if (fseek(fileBuf,total, SEEK_SET) != 0)
	{
		printf("fseek 2 fail");
		goto error;
	}

	if ((total = ftell(fileBuf)) == -1)
	{
		printf("ftell 3 fail");
	goto error;
	}
	//printf("total=%ld<br>",total);
	// truncate the terminating boundary line .
	int fd=fileno(fileBuf);
	ftruncate(fd,total);

	fflush(fileBuf);
	
	share_upload->uiWriteLen = len;
	
	
error:
	if (fileBuf != NULL)
	{
		fclose(fileBuf);
		if(chmod(filePath,S_IXUSR)!=0)
		{
		    printf("\ncm fail");
		    return;
		}
	}
	if(errorBuf[0]!='\0')
		//打印信息到网页的隐藏的iframe中
	//printf("<script type='text/javascript' language='javascript'>alert('%s');parent.location.replace('SystemE.html');</script>",errorBuf);
	printf("\nfail");
	else 
	{//printf("file upload success !<br>");
	//printf("<script type='text/javascript' language='javascript'>alert('File upload success!');parent.location.replace('SystemE.html');</script>");	
	printf("succeed");
	}

	share_upload->uiWriteLen = 0;

	return;
	
}


