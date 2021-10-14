#if 1
#include "dirent.h"
#include <wchar.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "sl_watchdog.h"
#if 1
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
	
	//share_upload = (struct shared_use_st*)shmat(shmid, (void*)0, 0);  
	share_upload = (SHARE_UPLOAD *)shmat(shmid, (void*)0, 0);  
	if(share_upload == NULL)
	{
		printf("shmat failed\n");	
		exit(0);  
	}  
	//printf("Memory attached at %X\n", (int)share_upload);  

    //share_mem->sm_eth_setting = eth_setting;
}


#define DEAL_BUF_LEN  1024
#define SIGN_CODE_LEN  100
#define FILE_NAME_LEN 64
#define FILE_SAVE_DIR "/tmp/update/"
#define FILE_NAME "web.tar.gz"
#define FILE_PATH_NAME "/tmp/update/web.tar.gz"
#define FILE_TAR_PATH "/bin/tar vxf /tmp/update/web.tar.gz -C /tmp/update/"
#define KILL_PROCESS "/bin/killall viu-vpu0h264enc-mdev-rtsp"

enum
{
    STATE_START,
    STATE_GET_SIGN_CODE,
    STATE_GET_FILE_NAME,
    STATE_GET_FILE_START,
    STATE_GET_FILE_CONTENT,
    STATE_CHECK_END,
    STATE_END
};

/***************************************************************************
ShowErrorInfo
****************************************************************************/
static void ShowErrorInfo(char * error)
{

    printf("Content-Type:text/html;charset=UTF-8\n\n");
    printf("<center><font color='red'>%s</font></center>" , error );
}

static void UploadFailed(void)
{
    printf("Content-Type:text/html\r\n\r\n");
	printf("failed");
}
/*主体从这里开始 */

int main(void)
{	
	system(KILL_PROCESS);
	
	//system("/bin/tar vxf /tmp/web.tar.gz -C /tmp/");
	//return 0;
#if 0
	int fd;
	int value, count;
	char buf[] = "hello"; 

	fd = open("/dev/silan-watchdog", O_RDWR);
	if( -1 == fd)
	{
		printf("open silan-watchdog failed\n");
		return 0;
	}

	value = 30;
	ioctl(fd, WDIOC_SETTIMEOUT, &value);
	ioctl(fd, WDIOC_GETTIMEOUT, &value);
	count = write(fd, buf, sizeof(buf)); //feed dog
	
#endif
        
#if 1
	int i = 0;
    FILE *fp; /* 文件指针，保存我们要获得的文件 */
    int getState = STATE_START;
    int contentLength;/*标准输入内容长度*/    
    int nowReadLen;
    int signCodeLen;
    int tmpLen;
    char *nowReadP;
    char *nowWriteP;
    char dealBuf[DEAL_BUF_LEN];
    char signCode[SIGN_CODE_LEN]; /*存储本次的特征码*/
    char tmpSignCode[SIGN_CODE_LEN];
    char fileName[FILE_NAME_LEN];
    char *fileName_prt;
    char *tmp;
    memset(dealBuf,0,DEAL_BUF_LEN);
    memset(signCode,0,SIGN_CODE_LEN);
    memset(fileName,0,FILE_NAME_LEN);
    nowReadLen = 0; 
    
    InitShareMemUpload();
	share_upload->uiWriteLen = 0;
   
    //ShowErrorInfo("start");
    tmp = getenv("CONTENT_LENGTH");
    //printf("Content-Type:text/html\r\n\r\n");
	//printf("%s", tmp); 
    if((char *)getenv("CONTENT_LENGTH")!=NULL)
    {
        contentLength = atoi((char *)getenv("CONTENT_LENGTH"));
        ShowErrorInfo("getenv!");
    }
    else
    {
        //printf()
        //ShowErrorInfo("Content length is null!");
        printf("Content-Type:text/html\r\n\r\n");
	    printf("succeed");
        //return 0;
//        UploadFailed();
        exit(1);
    }
    
    //return 0;
	share_upload->uiFileLen = contentLength;
	ShowErrorInfo("content length");
    //printf("Content-Type:text/html\r\n\r\n");
	//printf("succeed");
    
    while(contentLength > 0)
    {
        if(contentLength >= DEAL_BUF_LEN)
        {
            nowReadLen = DEAL_BUF_LEN;
        }
        else
        {
            nowReadLen = contentLength;
        }
        contentLength -= nowReadLen;
        if(fread(dealBuf,sizeof(char),nowReadLen,stdin) != nowReadLen)
        {
            ShowErrorInfo("Read content fail!");
            exit(1);
        }
        
        nowReadP = dealBuf;
        
        while(nowReadLen > 0)
        {
            switch (getState)
            {
                case STATE_START:
                    nowWriteP = signCode;
                    getState = STATE_GET_SIGN_CODE;
                case STATE_GET_SIGN_CODE:
                    if(strncmp(nowReadP,"\r\n",2) == 0)
                    {
                        signCodeLen = nowWriteP - signCode;
                        nowReadP++;
                        nowReadLen--;
                        *nowWriteP = 0;
                        getState = STATE_GET_FILE_NAME;
						ShowErrorInfo(signCode);
                    }
                    else
                    {
                        *nowWriteP = *nowReadP;
                        nowWriteP++;
                    }
                    break;
                case STATE_GET_FILE_NAME:
                    if(strncmp(nowReadP,"filename=",strlen("filename=")) == 0)
                    {
                        nowReadP += strlen("filename=");
                        nowReadLen -= strlen("filename=");
                        nowWriteP = fileName + strlen(FILE_SAVE_DIR);
                        while(*nowReadP != '\r')
                        {
                            if(*nowReadP == '\\' || *nowReadP == '/')
                            {
                                nowWriteP = fileName + strlen(FILE_SAVE_DIR);
                            }
                            else if(*nowReadP != '\"')
                            {
                                *nowWriteP = *nowReadP;
                                nowWriteP++;
                            }
                            nowReadP++;
                            nowReadLen--;
                        }
                        *nowWriteP = 0;
                        nowReadP++;
                        nowReadLen--;
                        getState = STATE_GET_FILE_START;
                        memcpy(fileName,FILE_SAVE_DIR,strlen(FILE_SAVE_DIR));

                       fileName_prt = fileName;

                       if(0>=strstr(fileName_prt,FILE_NAME))
                       {	
						   fprintf(stderr,"open file error\n");			   
						   exit(1);
					    }
                        
                        if((fp=fopen(fileName,"w"))==NULL)
                        {
                            fprintf(stderr,"open file error\n");
                            ShowErrorInfo("open file error\n");
                            exit(1);
                        }
						ShowErrorInfo(fileName);
                    }
                    break;
                case STATE_GET_FILE_START:
                    if(strncmp(nowReadP,"\r\n\r\n",4) == 0)
                    {
                        nowReadP += 3;
                        nowReadLen -= 3;
                        getState = STATE_GET_FILE_CONTENT;
						ShowErrorInfo("get");
                    }
                    
                    break;
                case STATE_GET_FILE_CONTENT:
                    if(*nowReadP != '\r')
                    {
                        fputc(*nowReadP,fp);
                    }
                    else
                    {
                        if(nowReadLen >= (signCodeLen + 2))
                        {
                            if(strncmp(nowReadP + 2,signCode,signCodeLen) == 0)
                            {
                                getState = STATE_END;
                                nowReadLen = 1;
                     
                            }
                            else
                            {
                                fputc(*nowReadP,fp);
                            }
                        }
                        else
                        {
                            getState = STATE_CHECK_END;
                            nowWriteP = tmpSignCode;
                            *nowWriteP = *nowReadP;
                            nowWriteP++;
                            tmpLen = 1;
                        }
                    }
                    break;
                case STATE_CHECK_END:
                    if(*nowReadP != '\r')
                    {
                        if(tmpLen < signCodeLen + 2)
                        {
                            *nowWriteP = *nowReadP;
                            nowWriteP++;
                            tmpLen++;
                            if(tmpLen == signCodeLen + 2)
                            {
                                *nowWriteP = 0;
                                if((tmpSignCode[1] == '\n')&&(strncmp(tmpSignCode + 2,signCode,signCodeLen) == 0))
                                {
                                    getState = STATE_END;
                                    nowReadLen = 1;
                                }
                                else
                                {
									//fprintf(fp,tmpSignCode);
                                    fwrite(tmpSignCode,sizeof(char),tmpLen,fp);
                                    getState = STATE_GET_FILE_CONTENT;
                                }
                            }
                        }
                    }
                    else
                    {
                        *nowWriteP = 0;
						//fprintf(fp,tmpSignCode);
                        fwrite(tmpSignCode,sizeof(char),tmpLen,fp);
                        nowWriteP = tmpSignCode;
                        *nowWriteP = *nowReadP;
                        nowWriteP++;
                        tmpLen = 1;
                    }
                    share_upload->uiWriteLen += tmpLen;
                    break;
                case STATE_END:
                    nowReadLen = 1;
                    break;
                default:
                    break;
            }
            nowReadLen--;
            nowReadP++;
            share_upload->uiWriteLen++;
        }
    }

    if(fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }
#endif

#if 0
	if(chmod(FILE_NAME, S_IRUSR|S_IXUSR|S_IWUSR|S_IXOTH|S_IROTH|S_IWOTH|S_IRGRP|S_IWGRP|S_IXGRP)!=0)
	{
		 printf("\ncm fail");
		 return 0;
	}
#endif
	
	share_upload->uiWriteLen = contentLength;

    //printf("Content-Type:text/html\r\n\r\n");
	//printf("succeed");
	system(FILE_TAR_PATH);
	//sleep(3);
	
	remove(FILE_PATH_NAME);
	
	printf("Content-Type:text/html\r\n\r\n");
	printf("succeed");

    return 0;
}
#endif 
#endif

#if 0
rtsp-mdev-vpu0-vpp_pv
 if(chmod("/tmp/viu-vpu0h264enc-mdev-rtsp", S_IRUSR|S_IXUSR|S_IWUSR|S_IXOTH|S_IROTH|S_IWOTH|S_IRGRP|S_IWGRP|S_IXGRP)!=0)
	chmod("/tmp/viu-vpu0h264enc-mdev-rtsp",S_IXUSR|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if(chmod("/tmp/viu-vpu0h264enc-mdev-rtsp",S_IXUSR)!=0)
	{
		 printf("\ncm fail");
		 return 0;
	}
  	
	  //system("rm /user/update/bin/viu-vpu0h264enc-mdev-rtsp");
      printf("content-Type:text/xml\n\n");
      system("/bin/killall viu-vpu0h264enc-mdev-rtsp");  
      if(remove("/tmp/viu-vpu0h264enc-mdev-rtsp")==0)
             printf("succeed\r\n\r\n");
      else
            printf("fail\r\n\r\n");

#endif
