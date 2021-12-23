#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/time.h>
#include <arpa/inet.h>                     
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <pthread.h>
#include <netinet/tcp.h>

//#include <sl_types.h>
//#include <errno.h>

#define SIGNALPORT  8100
#define DATAPORT    8101
#define LENGTH_OF_LISTEN_QUEUE     20
#define BUFFER_SIZE                300
#define DATANUM 10000000

static pthread_t app_rx_signal_ch_handler;
static pthread_t app_rx_data_ch_handler;

unsigned char * filebuf;
unsigned char * tempbuf;//set for recv not complete;


void *app_rx_signal_ch_main(void)
{
    int server_socket, new_server_socket;
        
    struct timezone tz;
    struct tcp_info info;
    int ret;
    
    unsigned char buf[BUFFER_SIZE];
    char filename[255]="";
    char syscmd[255]="";
    int iFileLen;

    
    int optlen = sizeof(struct tcp_info);

    
    // set socket's address information
    struct sockaddr_in   server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(SIGNALPORT);
    
    // create a stream socket
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }
    printf("Create Socket OK \n");

    //bind
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        printf("Server Bind Port: %d Failed!\n", SIGNALPORT);
        exit(1);
    }
    printf("bind socket ok \n");
    
//    sleep(5);
    
    // listen
    if (listen(server_socket, LENGTH_OF_LISTEN_QUEUE))
    {
        printf("Server Listen Failed!\n");
        exit(1);
    }
    printf("listen socket ok \n");

ReAccept:


        printf("Start Accept \n");
        new_server_socket = accept(server_socket, NULL,NULL);
        printf("Finish Accept \n");
        if (new_server_socket < 0)
        {
            printf("Server Accept Failed!\n");
            exit;
        }
        else
        {
            printf("Server Aceept OK \n");
        }



    int pos=0;
    int len=0;
    int i;
    
    struct timeval timeout={1,0};
    
    setsockopt(new_server_socket,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
    setsockopt(new_server_socket,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
    
    int tlen = 0;
    FILE* pFile;
    int result;
    unsigned char ucAckVal;
        
	while(1)
	{
        ucAckVal = 1;
	////////////////Recv Cmd/////////////////////////////////
	    len = recv(new_server_socket, buf, BUFFER_SIZE, 0);
        if(len < 0)
        {
            printf("Recv Cmd < 0 \n");
            close(new_server_socket);
            goto ReAccept;
        }
        if(len ==0)
        {
            printf("Recv Cmd len = 0 \n");
            close(new_server_socket);
                goto ReAccept;
        }
        if(len>0)
        {
            printf(" Recv len %d \n",len);
            
            printf("cmd = %d\n",buf[0]);
            
            if(buf[0]==11)//back door for command
            {
                unsigned char ucCmdLen = buf[1];
                memcpy(syscmd,buf+2,ucCmdLen);
            //    syscmd[ucCmdLen]='\0';
                printf("syscmd = %s\n",syscmd);

                buf[0]=11;
                buf[1]=1;
                len = send(new_server_socket,buf,2,0);
                if(len < 0)
                {
                    printf("Recv Cmd len < 0 \n");
                    close(new_server_socket);
                    goto ReAccept;
                }
                if(len ==0)
                {
                        printf("Recv Cmd len = 0 \n");
                        close(new_server_socket);
                          goto ReAccept;
                }
                
                sleep(2);
                system(syscmd);
                memset(syscmd,0,sizeof(syscmd));

                continue;
            }
            printf(" name len %d \n",buf[1]);
       
            for(i=2;i<buf[1]+2;i++)
            {
                filename[i-2]=buf[i];
                printf("%c",buf[i]);
            }    
            filename[i-2]='\0';
            printf("filename=%s\n",filename);
            //   for(i=0;i<len;i++)
            //       printf("%0x",buf[i]);
            printf("\n");
            
            printf("i start = %d, buf[i]=%0x,buf[i+1]=%0x\n",i,buf[i],buf[i+1]);
            iFileLen = buf[i];//+(int)buf[i+1]<<8+(int)buf[i+2]<<16+(int)buf[i+3]<<24;
            iFileLen += (int)buf[i+1]<<8;
            iFileLen += (int)buf[i+2]<<16;            
            iFileLen += (int)buf[i+3]<<24;            
            
            printf("iFileLen = %d\n",iFileLen);
            
            
            filebuf = (char * )malloc(sizeof(char) * iFileLen);

                        
            if(filebuf == NULL)
            {
                printf("file buf malloc error\n");
                ucAckVal=3;//NK
            }
            
            tempbuf = (char * )malloc(sizeof(char) * iFileLen);
            if(tempbuf == NULL)
            {
                printf("file buf malloc error\n");
                ucAckVal=3;//NK
            }


        }
            

        //reply ACK
        printf("send ready ACK \n");
        buf[0]=8;
        buf[1]=ucAckVal;
        len = send(new_server_socket,buf,2,0);
        if(len < 0)
        {
            printf("Recv Cmd len < 0 \n");
            close(new_server_socket);
            goto ReAccept;
        }
        if(len ==0)
        {
                printf("Recv Cmd len = 0 \n");
                close(new_server_socket);
                goto ReAccept;
        }

        if(ucAckVal>1)
        {
            close(new_server_socket);
            goto ReAccept;
        }
        
	////////////////Recv FileData/////////////////////////////////
	
        printf("Recv Start \n");
        tlen = 0;
	    while(tlen<iFileLen)
	    {
	        len = recv(new_server_socket, tempbuf, iFileLen, 0);
            if(len < 0)
            {
                printf("Recv file data len < 0 \n");
                close(new_server_socket);
                goto ReAccept;
            }
            if(len ==0)
            {
                    printf("Recv file data len = 0 \n");
                    close(new_server_socket);
                      goto ReAccept;
            }
            if(len>0)
            {
                printf("temp buf[0]=%0x\n",tempbuf[0]);

                printf("Recv file data len = %d, tlen = %d \n",len,tlen);
                memcpy(filebuf+tlen,tempbuf,len);
                tlen+=len;
            }
            
         }
        //recv all data then fwrite
        if(tlen == iFileLen)
        {
            if(strcmp(filename,"/customer/main_tx")==0)
            {
                system("/bin/killall main_tx");
                remove("/customer/main_tx");
            }
            
            if(strcmp(filename,"/customer/main_rx")==0)
            {
                system("/bin/killall main_rx");
                remove("/customer/main_rx");
            }

            //NULL!=strstr(filename,".sh"))

        	pFile = fopen(filename,"wb");
            if(pFile==NULL)
            {
                printf("file open issue \n");
                ucAckVal=2;//NK
            }
            else
            {
                result = fwrite(filebuf,1,tlen,pFile);
                if(result!=tlen)
                {
                    printf("result = %d\n",result);
                    printf("file write error\n");
                    ucAckVal = 2;
                }
                fclose(pFile);
            }
         }
         else
         {
            ucAckVal =5;
         }
         
         if(ucAckVal == 1)
         {
            printf("file wirte ok, tlen= %d, name=%s\n",tlen,filename);
            char command[255]="/bin/chmod 777 ";
            strcat(command,filename);
            system(command);
#if 1		 
          if(strcmp(filename,"/customer/main_tx")==0)
            {
                system("/bin/chmod 777 /customer/main_tx");
            }
            
            if(strcmp(filename,"/customer/main_rx")==0)
            {
                
                system("/bin/chmod 777 /customer/main_rx");
            }
			if(strcmp(filename,"tmp/run.sh")==0)
			{
				system("/bin/chmod 777 /tmp/run.sh");
			}
			
#endif			

         }
         free(filebuf);
         free(tempbuf);


  	////////////////Send Upgrade ACK/////////////////////////////////
        buf[0]=9;
        buf[1]=ucAckVal;
        len = send(new_server_socket,buf,2,0);
        if(len < 0)
        {
            printf("Send Cmd < 0 \n");
            close(new_server_socket);
            goto ReAccept;
        }
        if(len ==0)
        {
                printf("Send Cmd len = 0 \n");
                close(new_server_socket);
                  goto ReAccept;
        }
    //    sleep(5);
    //    system("reboot");
        	
    }//end of while

}


int main(int argc, char* argv[])
{
	int ret = -1;


	//SLOS_CreateMutex(&mutexlock);      

	ret = pthread_create(&app_rx_signal_ch_handler, NULL, app_rx_signal_ch_main, NULL);
	if (ret) {
		printf("Failed to Create rx signal Thread\n");
		return ret;
		}
	else
	{
	    printf("app_rx_signal_ch thread start ok\n");
	}
/*
	signal(SIGINT, signalHandle);
	signal(SIGTERM, signalHandle);
	signal(SIGIO, signalioHandle);
*/
	while(1)
	{
		sleep(1);
	}
	return 0;
}
