#include <sys/types.h>
#include <sys/socket.h>                    
#include <stdio.h>
#include <netinet/in.h>                    
#include <arpa/inet.h>                     
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <pthread.h>
#include <netinet/tcp.h>

#include <sys/time.h>
#include <errno.h>


#define UpgradePORT  8100
#define DATAPORT    8101
#define LENGTH_OF_LISTEN_QUEUE     20
#define BUFFER_SIZE                1024
#define DATANUM 1000

char * defTxName="/customer/main_tx";
char * defRxName="/customer/main_rx";

char infilename[255]="";
char tgfilename[255]="";
int iFileLen;
char * filebuf;  

/**************************
Command
00 08 update file info 
01 length of name;max 255
XX path+name
00 low byte of length of file
01
02
03 high byte of length of file

00 09 query update file status

ACK
00 08 
01 01 OK 02 Nk

00 09
01 01 OK 02 NK
**************************/

int app_tx_Upgrade_ch_main(char * address)  //8:info 9:query 11:system command
{
    int i;
    unsigned char buf[300];
    
    // set socket's address information
    struct sockaddr_in   server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(UpgradePORT);
    server_addr.sin_addr.s_addr = inet_addr(address);
    
    //jason add for change system time
    system("/sbin/sysctl -w net.ipv4.tcp_syn_retries=2");  //about 14s
	printf("net.ipv4.tcp_syn_retries reset ok \n");
     
    
    int iDataNum = 0;

    int sock_cli;
    sock_cli = socket(AF_INET,SOCK_STREAM, 0);

    printf("--------------------%s Upgrade Channel Connection start----------------------\n",address);
    
    if (connect(sock_cli, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Upgrade channel connect error \n");
        close(sock_cli);
        return -1;
    }
    else
    {
        printf("Upgrade channel Connected \n");
    }
    



    int pos=0;
    int len=0;

    
    struct timeval timeout={1,0};
  
    setsockopt(sock_cli,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
    setsockopt(sock_cli,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));

    {    
        buf[0] = 8; 
        buf[1] = strlen(tgfilename);
        for(i = 0;i<buf[1];i++)
            buf[i+2]=tgfilename[i];
        i=i+2;
        printf("tgfilename %s\n",tgfilename);
        buf[i]=iFileLen&0xff;
        buf[i+1]=(iFileLen>>8)&0xff;
        buf[i+2]=(iFileLen>>16)&0xff;
        buf[i+3]=(iFileLen>>24)&0xff;
        
        len = send(sock_cli, buf,buf[1]+6,0);
        if(len < 0)
        {
            printf("Send cmd len < 0 \n");
            printf("errno is %d\n",errno);
            close(sock_cli);
            return -2;
        }
        if(len ==0)
        {
                printf("Send cmd len = 0 \n");
                close(sock_cli);
                return -2;
        }
        if(len>0)
        {
            printf("filelen=%d\n",iFileLen);
            //printf("len = %d\n",len);
            /*
            for(i=0;i<len;i++)
                printf("%0x %0c\n",buf[i],buf[i]);
            */    
        }
        
        len = recv(sock_cli, buf, 2, 0);
        if (len < 0)
        {
            printf("Receive Ack fail, close connection\n");
            close(sock_cli);
            return -3;

        }
	    if (len ==0)
		{
		    printf("get len = 0 , network is broken\n");
		    close(sock_cli);
            return -3;
		}

        if(len>0)
        {
            //printf("len = %d\n",len);
			 printf("buf[0] = %d \n", buf[0]);
			 printf("buf[1]= %d\n",buf[1]);
			if(buf[0]==8&&buf[1]==1)
			    printf("Rx ready for upgrade\n");
			 else
			 {
			    printf("Rx not ready, terminate upgrade\n");
    		    close(sock_cli);
               return -4;			    
			 }

        }
    }
    
//send file data    
        printf("Send file data for upgrade \n");
        len = send(sock_cli,filebuf,iFileLen,0);
        if(len < 0)
        {
            printf("Send file data len < 0, upgrade fail\n");
            printf("errno is %d\n",errno);
            close(sock_cli);
            return -5;
        }
        if(len ==0)
        {
            printf("Send file data len = 0, upgrade fail\n");
            printf("errno is %d\n",errno);
            close(sock_cli);
            return -5;
        }
        if(len>0)
        {
            printf("send file data over, len = %d, wait for upgrade finish ACK \n",len);
        }

        sleep(5);
        len = recv(sock_cli, buf, 2, 0);
        if (len < 0)
        {
            printf("Receive Ack fail, len<0, close connection\n");
            close(sock_cli);
            return -6;
        }
	    if (len ==0)
		{
            printf("Receive Ack fail, len=0, close connection\n");
            close(sock_cli);
            return -6;
		}

        if(len>0)
        {
            //printf("len = %d\n",len);
			 printf("buf[0] = %d \n", buf[0]);
			 printf("buf[1]= %d\n",buf[1]);
			if(buf[0]==9&&buf[1]==1)
			    printf("Rx upgrade success\n");
			 else
			 {
			    printf("Rx upgrade fail,return buf[0]=%d,buf[1]=%d\n",buf[0],buf[1]);
    		    close(sock_cli);
               return -7;			    
			 }

        }        
        

//	free(buf);
    close(sock_cli);
    return 0;
}


 
int read_file()
{
    FILE *pFile;
    int result;
    
    printf("read file %s\n",infilename);
    pFile = fopen(infilename,"rb");
   
    if(pFile == NULL)
    {
        printf("file no exist \n");
        return -1;
    }
    fseek(pFile,0L,SEEK_END);
    iFileLen = ftell(pFile);
    printf("read filelen = %d\n",iFileLen);
    rewind(pFile);
    
    filebuf = (char * )malloc(sizeof(char) * iFileLen);
    if(filebuf == NULL)
    {
        printf("file buf malloc error\n");
        return -2;
    }
    
    result = fread(filebuf,1,iFileLen,pFile);
    if(result!=iFileLen)
    {
        printf("file read error\n");
        return -3;
    }
    fclose(pFile);
    return 0;
    
}
   
int main(int argc, char* argv[])
{

	char prefix[20]="192.168.36.";
	char add[20];
	char serial[3];
	unsigned char ucTxStart=0,ucTxEnd=0,ucRxStart=0,ucRxEnd=0;
	unsigned char ucSpec=0,ucSpecAdd=0;
	int i,ret;
	if(argc ==1)
	    printf("Usage: UpGradeServer [ALL|TX|RX|ADD|START-END] [source path+tgfilename] [target path+tgfilename]\n");
	if(argc > 1)
	{
	    if(strcmp(argv[1],"ALL")==0)
	    {
	        ucTxStart = 201;
	        ucTxEnd = 224;
	        ucRxStart = 1;
	        ucRxEnd = 128;
	        printf("all\n");
        }
        else if(strcmp(argv[1],"TX")==0)
	    {
	        ucTxStart = 201;
	        ucTxEnd = 224;
	        printf("tx\n");
        }
        
        else if(strcmp(argv[1],"RX")==0)
	    {
	        ucRxStart = 1;
	        ucRxEnd = 128;
	        printf("rx\n");
        }
        else if(strchr(argv[1],'-')!=NULL)
        {
            unsigned char ucStart,ucEnd;
            unsigned char ucArgvLen, ucSpePos;
            ucSpePos = strchr(argv[1],'-')-argv[1];
            ucArgvLen = strlen(argv[1]);
            //printf("Pos=%d,Len=%d\n",ucSpePos,ucArgvLen);
            memcpy(add,argv[1],ucSpePos);
            add[ucSpePos]='\0';
            /*
            printf("argv=%x\n",argv[1][0]);
            printf("argv=%x\n",argv[1][1]);
            printf("argv=%x\n",argv[1][2]);  
            printf("add=%x\n",add[0]);
            printf("add=%x\n",add[1]);
            printf("add=%x\n",add[2]);            
            */
            //printf("3\n");            

            ucStart=atoi(add);
            strcpy(add,"");

            memcpy(add,argv[1]+ucSpePos+1,ucArgvLen-ucSpePos);
            add[ucArgvLen-ucSpePos]='\0';
            /*
            printf("argv=%x\n",argv[1][ucSpePos+0]);
            printf("argv=%x\n",argv[1][ucSpePos+1]);
            printf("argv=%x\n",argv[1][ucSpePos+2]);  
            printf("add=%x\n",add[0]);
            printf("add=%x\n",add[1]);
            printf("add=%x\n",add[2]);             
            */
            ucEnd = atoi(add);
            strcpy(add,"");
            //printf("start = %d, end = %d\n",ucStart,ucEnd);
            if(ucStart>=1&&ucEnd<=96&&ucEnd>ucStart)
            {
                printf("Update Rx from %d to %d \n",ucStart,ucEnd);
                ucRxStart=ucStart;
                ucRxEnd=ucEnd;         
            }
            else if(ucStart>=201&&ucEnd<=224&&ucEnd>ucStart)
            {
                printf("Update Tx from %d to %d \n",ucStart,ucEnd);
                ucTxStart=ucStart;
                ucTxEnd=ucEnd;         
            }
            else
            {
                printf("Input No. is wrong, not in correct TX or RX No. area or start No. > end No.(End No. should > Start No.)\n");
                return 0;
            }
            
        }
        else
        {
            ucSpec=1;
            //strcpy(add,argv[1]);
            //printf("%s\n",add);
        }
	}
	if(argc == 3) //infilename same as tgfilename
	{
	    strcpy(infilename, argv[2]);
	    strcpy(tgfilename, argv[2]);
	    printf("%s\n",tgfilename);
            if(read_file()<0)
            {
                printf("file error\n");
                return 0;
            }

	}
	if(argc == 4) //infilename different with tgfilename
	{
	    strcpy(infilename, argv[2]);
	    strcpy(tgfilename, argv[3]);
	    printf("%s\n",tgfilename);
            if(read_file()<0)
            {
                printf("file error\n");
                return 0;
            }

	}
	
	if(ucTxStart > 0)
    {
        if(argc == 2)//no spec file
        {
            strcpy(infilename,defTxName);
            strcpy(tgfilename,defTxName);
            if(read_file()<0)
            {
                printf("file error\n");
                return 0;
            }

        }
        
        for(i=ucTxStart;i<=ucTxEnd;i++)
        {
            strcpy(add,"");
            strcat(add,prefix);
            sprintf(serial,"%d",i);
            strcat(add,serial);
            //printf("%s\n",add);

            ret = app_tx_Upgrade_ch_main(add);
            if(ret<0)
            {
                printf("Device %s Upgrade fail \n",add);
                continue;
            }
            else
            {
                printf("Device %s Upgrade success \n",add);
            }
            
                      
        }
    
    }
	if(ucRxStart > 0)    	
	{
	    if(argc == 2)//no spec file
        {
            strcpy(infilename,defRxName);
            strcpy(tgfilename,defRxName);
            if(read_file()<0)
            {
                printf("file error\n");
                return 0;
            }

        }
        
        for(i=ucRxStart;i<=ucRxEnd;i++)
        {
            strcpy(add,"");
            strcat(add,prefix);
            sprintf(serial,"%d",i);
            strcat(add,serial);
            //printf("%s\n",add);

            ret = app_tx_Upgrade_ch_main(add);
            if(ret<0)
            {
                printf("Device %s Upgrade fail \n",add);
                continue;
            }
            else
            {
                printf("Device %s Upgrade success \n",add);
            }
        }	
	}
	
	if(ucSpec > 0)    	
	{
	    strcpy(add,"");
        strcat(add,prefix);
	    strcat(add,argv[1]);
	    
	    ucSpecAdd = atoi(argv[1]);
	    if(ucSpecAdd>0 && ucSpecAdd<129)
	    {
    	    if(argc == 2)//no spec file
    	    {
	            strcpy(infilename,defRxName);
	            strcpy(tgfilename,defRxName);
                if(read_file()<0)
                {
                    printf("file error\n");
                    return 0;
                }

	        }
	    }
	    else if(ucSpecAdd>200 && ucSpecAdd<225)
	    {
    	    if(argc == 2)//no spec file
    	    {
	            strcpy(infilename,defTxName);
	            strcpy(tgfilename,defTxName);	            
                if(read_file()<0)
                {
                    printf("file error\n");
                    return 0;
                }

	        }
	    }
        if(read_file()<0)
        {
            printf("file error\n");
            return 0;
        }

	    ret = app_tx_Upgrade_ch_main(add);
            if(ret<0)
            {
                printf("Device %s Upgrade fail \n",add);
            }
            else
            {
                printf("Device %s Upgrade success \n",add);
            }
	}

    free(filebuf);
	return 0;
}
