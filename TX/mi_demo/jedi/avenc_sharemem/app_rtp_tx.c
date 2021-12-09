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
#include "linux_common.h"
#include "video_ringfifo.h"
//#include "ring_buffer.h"
#include "crc.h"
#include "sharemem.h"
#include "hdmi_info.h"

#define NO_SIGNAL	"check hdmi signal"
#define CANT_SUPPORT 	"can't support the signal"

//#define OUTPUT_H264
//#define OUTPUT_PCM
#define CHECKSUM_AUDIO
#define CHECKSUM_VIDEO
extern unsigned char g_Exit;
static FILE *pcmfp = NULL;
static FILE *pcmfp1 = NULL;
static FILE *p264File = NULL;

DATAHEAD DataHead;
char multicast[20] = "239.255.42.31";
unsigned char *pCheck="0abc"; //send flag with HDMI check signal

#if 1
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
	char pCheck[30] = {}; //send flag with HDMI check signal
	unsigned char *pBuf = (unsigned char*)malloc(sizeof(unsigned char)*BUFFER_SIZE);
	unsigned char *dst;
	unsigned char *pStream;
	int ringbuflen=0;
	RING_NODE_FRAME FrameInfo;
	int send_len=0;
	struct timezone tz;
	struct timeval tv1, tv2;
	unsigned int timeOut;

#ifdef OUTPUT_PCM
	pcmfp = OpenFile("./send.pcm", "w+");
	pcmfp1 = OpenFile("./send1.pcm", "w+");
#endif

#ifdef OUTPUT_H264
	FILE *p264File = OpenFile("./test.264", "w+");
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
	//server_addr.sin_addr.s_addr=inet_addr(multicast); //multicast  
	server_addr.sin_addr.s_addr=inet_addr(share_mem->sm_eth_setting.strEthMulticast); //multicast 
	//printf("server_addr.sin_addr.s_addr : %s \n", share_mem->sm_eth_setting.strEthMulticast);
	
	sock_cli = CreateUDPSocket();
	if (sock_cli < 0){
		sleep(1);
		goto ReSocket;
	}

	//SetReuseSocketAddr(sock_cli);
	//SetSendTimeOut(sock_cli, 2, 0);

	DataHead.iProbe = 0x1A1B1C1D;
	DataHead.uSeq = 0;
	DataHead.iTimeStamp = 0;

#endif
	while (g_Exit)
	{
		//multicast switch 
#if 0
		//Check HDMI signal 
		char hdmi_state = check_hdmi();
		if (0 < hdmi_state)
		{
			timeOut++;
			bzero(pCheck, sizeof(pCheck));
			if (-1 == hdmi_state)
			{
				memcpy(pCheck, NO_SIGNAL, sizeof(pCheck));
				printf(NO_SIGNAL);
			}
			else if (2 == hdmi_state)
			{
				memcpy(pCheck, CANT_SUPPORT, sizeof(pCheck));
				printf(CANT_SUPPORT);
			}
			SendToDestAddr(sock_cli, pCheck, sizeof(pCheck), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
			
			sleep(1);
			if (timeOut > 3000)
			{
				//reboot1();
			}
			continue;
		}
		else
		{
			timeOut = 0;
		}
#endif
	
#if 1
		send_len=0;

		if (GetAllNumOfElements() > 0)
		{
			#if 0
			ret = gettimeofday(&tv1,&tz);
			if (ret < 0){
				printf("printf_log gettimeofday error \n");
				perror("gettimeofday");
			}
			
			DataHead.uSeq += 1;
			DataHead.iTimeStamp = tv1.tv_sec-144221050; 
			//printf("time %u:%u\n",tv1.tv_sec,tv1.tv_usec);
			#endif

			GetElementFromRing(&FrameInfo);
			printf("V");
			if (FrameInfo.FrameType == FRAME_TYPE_I) //I frame
			{
				//printf("this is i frame\n");
				printf("-I-\n\n");
				DataHead.iLen = FrameInfo.FrameSize;
				DataHead.uPayloadType = H264;
				tmp_len = FrameInfo.FrameSize;
				dst = (unsigned char *)FrameInfo.pFrame;
#ifdef CHECKSUM_VIDEO
				if (DataHead.iLen < CHECK_SUM_COUNT)
				{
					DataHead.usChecksum = csum((unsigned char *)dst, DataHead.iLen); //check sum
				}
				else
				{
					DataHead.usChecksum = csum((unsigned char *)dst, CHECK_SUM_COUNT); 
				}
#endif
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
			else //p frame
			{
				printf("P");
				//printf("this is p frame\n");
				tmp_len = FrameInfo.FrameSize;
				DataHead.iLen = FrameInfo.FrameSize;
				DataHead.uPayloadType = H264;
				dst = (unsigned char *)FrameInfo.pFrame;
				//printf("Ptmp_len=%d\n",tmp_len);
#ifdef CHECKSUM_VIDEO
				if (DataHead.iLen < CHECK_SUM_COUNT)
				{
					DataHead.usChecksum = csum((unsigned char *)dst, DataHead.iLen); //check sum
				}
				else
				{
					DataHead.usChecksum = csum((unsigned char *)dst, CHECK_SUM_COUNT);
				}
#endif
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
		} /*end video judge*/
#endif
		/***************Audio data****************/		
#if 1
		if (RingGetByteStreamMemoryBufferCnt() > FETCH_COUNT)
		{
			printf("A");
			//printf("/***************Audio data****************/\n");
			#if 0
			ret = gettimeofday(&tv1,&tz);
			if (ret < 0){
				printf("printf_log gettimeofday error \n");
				perror("gettimeofday");
			}
			DataHead.iTimeStamp = tv1.tv_sec-144221050;
			#endif

			DataHead.uPayloadType = WAV;
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
		} /*end of audio judge*/
#endif
	} //end of while (1)

    close(sock_cli);
	free(dst);
	
    return 0;
}
