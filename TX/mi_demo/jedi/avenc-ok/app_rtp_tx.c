#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include "app_rtp_tx.h"
#include "common.h"
#include "video_ringfifo.h"
//#include "ring_buffer.h"

//#define OUTPUT_H264
#define OUTPUT_PCM
//#define CHECKSUM_AUDIO
//#define CHECKSUM_VIDEO

static FILE *pcmfp = NULL;
static FILE *pcmfp1 = NULL;
static FILE *p264File = NULL;

DATAHEAD DataHead;
char multicast[20] = "239.255.42.01";
unsigned char *pCheck="0abc"; //send flag with HDMI check signal

#if 0
static unsigned short csum(unsigned char *buf, int nwords)
{
	//unsigned long sum;
	unsigned short check;
	
	check = crc_check(buf, nwords);
	//usleep(1000);
	return (unsigned short)(~check);
}
#endif

size_t Write264ToFile(const void *ptr, size_t size, size_t nmemb,
    FILE *stream)
{
#ifdef OUTPUT_H264
	WriteToFile(ptr, size, nmemb, stream);
#endif
}

size_t WritePCMToFile(const void *ptr, size_t size, size_t nmemb,
    FILE *stream)
{
#ifdef OUTPUT_PCM
	WriteToFile(ptr, size, nmemb, stream);
#endif
}

void *send_rtp_tx(void)
{
	int ret,tmp_len,len;
	unsigned char *pCheck="0abc"; //send flag with HDMI check signal
	unsigned char *pBuf = (unsigned char*)malloc(sizeof(unsigned char)*BUFFER_SIZE);
	unsigned char *dst;
	unsigned char *pStream;
	int ringbuflen=0;
	RING_NODE_FRAME FrameInfo;
	int send_len=0;
	struct timezone tz;
	struct timeval tv1, tv2;

#ifdef OUTPUT_PCM
	OpenFile(pcmfp, "./send.pcm", "w+");
	OpenFile(pcmfp1, "./send1.pcm", "w+");
#endif

#ifdef OUTPUT_H264
	FILE *p264File = fopen("./test.264", "w+");
#endif
	//set socket's address information
	int sock_cli;
	socklen_t servlen_addr_length;
    struct sockaddr_in server_addr;
    servlen_addr_length = sizeof(server_addr);
#if 1
ReSocket:
	memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(RTP_PORT);     //8001
	server_addr.sin_addr.s_addr=inet_addr(multicast); //multicast  
	
	sock_cli = CreateUDPSocket();
	if (sock_cli < 0){
		sleep(1);
		goto ReSocket;
	}

	SetReuseSocketAddr(sock_cli);
	SetSendTimeOut(sock_cli, 2, 0);

	DataHead.iProbe = 0x1A1B1C1D;
	DataHead.uSeq = 0;
	DataHead.iTimeStamp = 0;

#endif
	while (1)
	{
		send_len=0;

		ringbuflen = GetElementFromRing(&FrameInfo);
        if(ringbuflen == 0)
        {
			usleep(5000);
			continue ;
		}
#if 1
		else
		{
			#if 1
			ret = gettimeofday(&tv1,&tz);
			if (ret < 0){
				printf("printf_log gettimeofday error \n");
				perror("gettimeofday");
			}
			#endif
			
			DataHead.uSeq += 1;
			DataHead.iTimeStamp = tv1.tv_sec-144221050; 
			//printf("time %u:%u\n",tv1.tv_sec,tv1.tv_usec);

			if (FrameInfo.FrameType == FRAME_TYPE_I) //I frame
			{
				//printf("this is i frame\n");
				DataHead.iLen = FrameInfo.FrameSize;
				DataHead.uPayloadType = H264;
				tmp_len = FrameInfo.FrameSize;
				dst = (unsigned char *)FrameInfo.pFrame;

				//send Data Header
				SendToDestAddr(sock_cli, &DataHead, sizeof(DataHead), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

				//send I frame data header
				while (tmp_len > UDP_MTU)	
				{
					len = SendToDestAddr(sock_cli, dst, UDP_MTU, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
					if (len > 0)
					{
						Write264ToFile((unsigned char *)dst, UDP_MTU, 1, p264File);
						tmp_len -= UDP_MTU;
						dst += UDP_MTU;
						send_len+=len;
					}
				}
				len = SendToDestAddr(sock_cli, dst, tmp_len, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));	
				if (len > 0)
				{
					send_len+=len;
					Write264ToFile((unsigned char *)dst, tmp_len, 1, p264File);
				}
			}
			else//p frame
			{
				//printf("this is p frame\n");
				tmp_len = FrameInfo.FrameSize;
				DataHead.iLen = FrameInfo.FrameSize;
				DataHead.uPayloadType = H264;
				dst = (unsigned char *)FrameInfo.pFrame;
				//printf("Ptmp_len=%d\n",tmp_len);
				
				//send Data Header
				len = SendToDestAddr(sock_cli, &DataHead, sizeof(DataHead), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
				while (tmp_len > UDP_MTU)
				{
					len = SendToDestAddr(sock_cli, dst, UDP_MTU, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
					if (len > 0)
					{
						send_len+=len;
						Write264ToFile((unsigned char *)dst, UDP_MTU, 1, p264File);
						tmp_len -= UDP_MTU;
						dst += UDP_MTU;
					}
				}
				len = SendToDestAddr(sock_cli, dst, tmp_len, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
				if (len > 0)
				{
					send_len+=len;
					Write264ToFile((unsigned char *)dst, tmp_len, 1, p264File);				
				}
			}
		}
#endif
		/***************Audio data****************/
#if 1
		if (RingGetByteStreamMemoryBufferCnt() > FETCH_COUNT)
		{
			//printf("/***************Audio data****************/\n");
			ret = gettimeofday(&tv1,&tz);
			if (ret < 0){
				printf("printf_log gettimeofday error \n");
				perror("gettimeofday");
			}
			DataHead.uPayloadType = WAV;
			DataHead.iTimeStamp = tv1.tv_sec-144221050;
			DataHead.iLen = FETCH_COUNT;
			
			RingPullFromByteStreamMemoryBuffer(pBuf, FETCH_COUNT);
			//WritePCMToFile(pBuf, FETCH_COUNT, 1, pcmfp1);
			tmp_len = FETCH_COUNT;
			pStream = (unsigned char *)pBuf;//(unsigned char *)dst;

#ifdef CHECKSUM_AUDIO
			DataHead.usChecksum = csum(pStream, FETCH_COUNT);
#endif
			//send Data Header
			len = SendToDestAddr(sock_cli, &DataHead, sizeof(DataHead), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
			//loop send audio data
			while (tmp_len > UDP_MTU)
			{
				len = SendToDestAddr(sock_cli, pStream, UDP_MTU, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
				//WritePCMToFile(pStream, UDP_MTU, 1, pcmfp);
				tmp_len -= UDP_MTU;
				pStream += UDP_MTU;
			}
			len = SendToDestAddr(sock_cli, pStream, tmp_len, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
			//WritePCMToFile(pStream, tmp_len, 1, pcmfp);
		}
		else
		{
			//printf("ringbuffer not full \n");
			usleep(100);
		}
#endif
	} //end of while (1)

	printf(" break while(1)   ,endif \n");
    close(sock_cli);
	free(dst);
	
    return 0;
}

#if 0
void *send_tcp_tx(void)
{
	printf("send_tcp_tx=========\n");
	
	const int opt = -1;
	int sock_cli;
	socklen_t servlen_addr_length;
    struct sockaddr_in servaddr;
    servlen_addr_length = sizeof(servaddr);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8888);
	servaddr.sin_addr.s_addr = inet_addr("192.168.1.166"); //TCP
ReSocket:
	sock_cli = socket(AF_INET,SOCK_STREAM, 0); //TCP
	if (sock_cli < 0)
    {
		perror("socket");
		sleep(1);
		goto ReSocket;
	}
	
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("8001 data channel connect error \n");
        perror("connect");
        sleep(1);
        close(sock_cli);
        goto ReSocket;
    }
    printf("8001 data channel Connected \n");	
	
	struct timezone tz;
	struct timeval tv1, tv2;
	struct timeval timeout = {2,0};
	setsockopt(sock_cli,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
	setsockopt(sock_cli,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
	
	
	DataHead.iProbe = 0x1A1B1C1D;
	DataHead.uSeq = 0;
	DataHead.iTimeStamp = 0;
	
	int ringbuflen=0;
	struct ringbuf ringinfo;
	int ret,tmp_len,len;
	unsigned char *buf = (unsigned char*)malloc(sizeof(unsigned char)*BUFFER_SIZE);
	unsigned char *dst;
	int alllong=0;
	
	while(1)
	{
		ringbuflen = ringget(&ringinfo);
		//printf("ringbuflen=%d\n",ringbuflen);
        if(ringbuflen ==0)
        {
			usleep(1000);
			continue ;
		}
		else
		{
			//printf("all long =%d\n",alllong);
			ret = gettimeofday(&tv1,&tz);
			if (ret < 0){
				printf("printf_log gettimeofday error \n");
				perror("gettimeofday");
			}
			
			DataHead.uSeq += 1;
			DataHead.iTimeStamp = tv1.tv_sec-144221050;  
			
			if(ringinfo.frame_type ==FRAME_TYPE_I)	
			{
				DataHead.uPayloadType = H264;
				DataHead.iLen = ringinfo.size;
				tmp_len = ringinfo.size;
				dst = (unsigned char *)ringinfo.buffer;
				//printf("Itmp_len=%d\n",tmp_len);
				
				//send Data Header
				len = send(sock_cli, &DataHead, sizeof(DataHead), 0);
				if(len <= 0)
				{
					printf("Send data len <= 0 len= %d \n",len);
					printf("errno is %d\n",errno);
					perror("sendto");
					//close(sock_cli);
					//goto ReSocket;
				}
				
				//send I frame data 
				while (tmp_len > UDP_MTU)	
				{
					len = send(sock_cli, dst, UDP_MTU, 0);
					if(len <= 0)
					{
						printf("Send idata len <= 0 len= %d \n",len);
						perror(send);
						printf("errno is %d\n",errno);
						//close(sock_cli);
						//goto ReSocket;
					}
					else
					{
						//alllong+=len;	
						//recv(sock_cli, 0, 1, 0);
						tmp_len -= UDP_MTU;
						dst += UDP_MTU;
					}
				}
				len = send(sock_cli, dst, tmp_len, 0);
				if(len <= 0)
				{
					printf("Send Idata len <= 0 len= %d \n",len);
					printf("errno is %d\n",errno);
					perror(send);
					//close(sock_cli);
					//goto ReSocket;
				}	
				else
				{
					//alllong+=len;
				}
				//recv(sock_cli, 0, 1, 0);
			}
			else					//p frame
			{
				//printf("this is pframe\n");
				DataHead.iLen = ringinfo.size;
				DataHead.uPayloadType = H264;
				tmp_len = ringinfo.size;		
				dst = (unsigned char *)ringinfo.buffer;
				//printf("Ptmp_len=%d\n",tmp_len);	
				
				//send Data Header
				len = send(sock_cli, &DataHead, sizeof(DataHead), 0);
				if(len <= 0)
				{
					printf("Send data len <= 0 len= %d \n",len);
					printf("errno is %d\n",errno);
					perror("sendto");
					//close(sock_cli);
					//goto ReSocket;
				}
				
				while (tmp_len > UDP_MTU)	
				{
					len = send(sock_cli, dst, UDP_MTU, 0);
					if(len <= 0)
					{
						printf("Send pdata len <= 0 len= %d \n",len);
						printf("errno is %d\n",errno);
						//close(sock_cli);
						//goto ReSocket;
					}	
					else
					{
						//alllong+=len;
						tmp_len -= UDP_MTU;
						dst += UDP_MTU;
					}
					//recv(sock_cli, 0, 1, 0);
				}
				len = send(sock_cli, dst, tmp_len, 0);
				if(len <= 0)
				{
					printf("Send pdata len <= 0 len= %d \n",len);
					printf("errno is %d\n",errno);
					//close(sock_cli);
					//goto ReSocket;
				}
				else
				{
					//alllong+=len;
				}
				//recv(sock_cli, 0, 1, 0);	
			}
			
		}
		
	}
	
}
#endif