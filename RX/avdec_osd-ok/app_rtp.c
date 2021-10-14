#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <signal.h>
#include "app_rtp.h"

//#define RTP_UDP
//#define CHECKSUM_AUDIO
//#define CHECKSUM_VIDEO

static FILE * outfile  = NULL;

static unsigned char uNowPayloadType;

char multicast[20] = "239.255.42.01";
char idr_flag = 0;
char outfilename[128] = "./test2.264";
FILE *p264File;

unsigned short int usAudioChecksum;
unsigned short int usVideoChecksum;
#if 0
static unsigned short csum(unsigned char *buf, int nwords)
{
	//unsigned long sum;
	unsigned short check;
	
	check = crc_check(buf, nwords);
	
	return (unsigned short)(~check);
}
#endif
extern void wacPushFrameToMDev(unsigned char *pFrame, unsigned int uiSize);

extern void audio_play(unsigned char *pPCM, unsigned int uiSize);

static int process_start_frame(unsigned char *buf, unsigned int len, unsigned int *piCpySize, unsigned int *piFrameLeftLen)
{
    DATAHEAD *pDataHead = (DATAHEAD *)buf;
    unsigned int DataFrameLen, iPacketLeftLen,size;
    
    if(pDataHead->iProbe != 0x1A1B1C1D)
    {
        return -1;
    }
	
    //printf("Find probe \n");
    //printf("Find len = %d \n",pDataHead->iLen);
	uNowPayloadType = pDataHead->uPayloadType;
	DataFrameLen = pDataHead->iLen;
	iPacketLeftLen = len;
	
#ifdef CHECKSUM_AUDIO
	if (H264 == uNowPayloadType)
	{
		//printf("\n------------------------%d----------------------\n", pDataHead->uSeq);
		//printf("DataFrameLen : %d \n", DataFrameLen);
		usVideoChecksum = pDataHead->usChecksum;
	}
	if (WAV == uNowPayloadType)
	{
		//printf("\n*****************************************\n");
		//printf("DataFrameLen : %d \n", DataFrameLen);
		usAudioChecksum = pDataHead->usChecksum;
	}
#endif
	while(DataFrameLen < (iPacketLeftLen-sizeof(DATAHEAD))) //multiframe in one packet
	{
		//copy the frame
		//printf("copy process 11, len = %d \n",DataFrameLen);
		memcpy(pFrame,buf+sizeof(DATAHEAD),DataFrameLen);
		
		//process the frame
		size = DataFrameLen;
		//printf("process the frame frame len = %d \n", DataFrameLen);
		if (H264 == uNowPayloadType)
		{
			//printf("\n----------------H264 1-------------- \n");
			//printf("wac process 1, size = %d \n",size);
#ifdef CHECKSUM_VIDEO
			if (size < CHECK_SUM_COUNT)
			{
				if (csum(pFrame, size) == usVideoChecksum)
				{
					wacPushFrameToMDev(pFrame,size);
				}
				else
				{
					printf("video checksum error !\n");
				}
			}
			else
			{
				if (csum(pFrame, CHECK_SUM_COUNT) == usVideoChecksum)
				{
					wacPushFrameToMDev(pFrame,size);
				}
				else
				{
					printf("video checksum error !\n");
				}
			}
			
#else
			wacPushFrameToMDev(pFrame,size);
#endif
			//fwrite(pFrame, size, 1, outfile);
		}
		if (WAV == uNowPayloadType)
		{
			//printf("\n***************WAV 1*****************\n");
			//printf("size : %d \n", size);
#ifdef CHECKSUM_AUDIO
			printf("sum: %x \n", csum(pFrame, size));
			printf("usAudioChecksum : %x \n", usAudioChecksum);
			if (csum(pFrame, size) == usAudioChecksum)
			{
				audio_play(pFrame, size);
			}
			else
			{
				printf("audio checksum error !\n");
			}
#else
			audio_play(pFrame, size);
#endif
			//fwrite(pFrame, size, 1, outfile);
		}
		
		//find next head
		iPacketLeftLen = iPacketLeftLen - (DataFrameLen+sizeof(DATAHEAD));
		buf = buf+DataFrameLen+sizeof(DATAHEAD);
		pDataHead = buf;
#ifdef CHECKSUM_AUDIO
		if (H264 == uNowPayloadType)
		{
			//printf("\n------------------------%d----------------------\n", pDataHead->uSeq);
			//printf("DataFrameLen : %d \n", DataFrameLen);
			usVideoChecksum = pDataHead->usChecksum;
		}
		if (WAV == uNowPayloadType)
		{
			//printf("\n*****************************************\n");
			//printf("DataFrameLen : %d \n", DataFrameLen);
			usAudioChecksum = pDataHead->usChecksum;
			//printf("usAudioChecksum: %d \n",usAudioChecksum )
			
		}
#endif
		if(iPacketLeftLen >= sizeof(DATAHEAD))
		{
			if(0x1A1B1C1D == pDataHead->iProbe)
			{
				uNowPayloadType = pDataHead->uPayloadType;
				DataFrameLen = pDataHead->iLen;
				//printf("PayloadType : %d \n", pDataHead->uPayloadType);
				//printf("iProbe = %0x \n",pDataHead->iProbe);
				//printf("iTimeStamp : %d \n", pDataHead->iTimeStamp);
				//printf("uSeq : %d \n", pDataHead->uSeq);
				//printf("Find probe \n");
				//printf("Payload Type : %d \n", pDataHead->uPayloadType);
				//printf("Find len = %d \n",pDataHead->iLen);
			}
			else
			{
				printf("Packet wrong, searching again \n");
				//uFrameErrorFlag = 1;
				return -1;
			}
		}
		else //data head split in two packet
		{
			printf("/////////////////////////////error 2 //////////////////////////////");
			printf(" Data Head in two packet, will do it in future \n");
			//uFrameErrorFlag = 1;
			return -1;
		}
	}
	
	//process last part of packet
	if(DataFrameLen > (iPacketLeftLen-sizeof(DATAHEAD))) // not finished
	{
		//printf("process DataFrameLen: %d \n", DataFrameLen);
		//copy frame data to pFrame;
		
		//copy the frame
		memcpy(pFrame,buf+sizeof(DATAHEAD),iPacketLeftLen-sizeof(DATAHEAD));
		*piCpySize = iPacketLeftLen-sizeof(DATAHEAD);
		*piFrameLeftLen = DataFrameLen-(iPacketLeftLen-sizeof(DATAHEAD));
		//printf("*piCpySize : %d \n", *piCpySize);
		//printf("*piFrameLeftLen : %d \n", *piFrameLeftLen);
		
		return 1;
	}
	else if( DataFrameLen == (iPacketLeftLen-sizeof(DATAHEAD))) //finished
	{
		//eNextPacketType = START_PACKET;
		//copy the frame
		//printf("copy process 14, len = %d \n",DataFrameLen);
		memcpy(pFrame,buf+sizeof(DATAHEAD),DataFrameLen);
		//process the frame
		size = DataFrameLen;
		//printf("process the frame frame len = %d \n", DataFrameLen);
		if (H264 == uNowPayloadType)
		{
			//printf("\n----------------H264 2-------------- \n");
			//printf("wac process 2, size = %d \n",size);
#ifdef CHECKSUM_VIDEO
			if (size < CHECK_SUM_COUNT)
			{
				if (csum(pFrame, size) == usVideoChecksum)
				{
					wacPushFrameToMDev(pFrame,size);
				}
				else
				{
					printf("video checksum error !\n");
				}
			}
			else
			{
				if (csum(pFrame, CHECK_SUM_COUNT) == usVideoChecksum)
				{
					wacPushFrameToMDev(pFrame,size);
				}
				else
				{
					printf("video checksum error !\n");
				}
			}
#else
			wacPushFrameToMDev(pFrame,size);
#endif
			//fwrite(pFrame, size, 1, outfile);
		}
		if (WAV == uNowPayloadType)
		{
			//printf("\n***************WAV 2*****************\n");
			printf("size : %d \n", size);
#ifdef CHECKSUM_AUDIO
			printf("sum: %d \n", csum(pFrame, size));
			printf("usAudioChecksum : %d \n", usAudioChecksum);
			if (csum(pFrame, size) == usAudioChecksum)
			{
				audio_play(pFrame, size);
			}
			else
			{
				printf("audio checksum error !\n");
			}
#else
			audio_play(pFrame, size);
#endif
			//fwrite(pFrame, size, 1, outfile);
		}
		
		return 0;
	}
}

#if 1
//UDP
void *app_rtp_main(void *args)
{
	int ret, len, timeOut = 0;
	//p264File = fopen("./test.264", "w+");

#if 1
	//int server_socket, new_server_socket;
	int rtp_server_socket;
	struct sockaddr_in server_addr,  client_addr;
	socklen_t clielen_addr_length;
	clielen_addr_length = sizeof(client_addr);
	
ReSocket:
	//struct tcp_info info;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(RTP_PORT);
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	
	//create a stream socket
	rtp_server_socket = socket(AF_INET, SOCK_DGRAM, 0); //UDP
	if (rtp_server_socket < 0)
	{
		printf("Create Socket Failed!\n");
		perror("socket");
		sleep(1);
		goto ReSocket;
	}

    //printf("Create Socket OK \n");
    int on=1;
    setsockopt(rtp_server_socket, SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    //bind
    if (bind(rtp_server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        printf("Server Bind Port: 8001 Failed!\n");
        perror("bind");
        close(rtp_server_socket);
        sleep(1);
        goto ReSocket;
    }
    //printf("bind socket ok \n");
#endif
	
#if 0
	//set socket broadcast
	const int opt = -1;
	ret = setsockopt(rtp_server_socket,SOL_SOCKET,SO_BROADCAST,(char*)&opt,sizeof(opt));
	if(ret < 0)
	{
		printf("set broadcast error \n");
		perror("setsockopt");
	}
#endif

#if 1
	//set Multicast loop
	int loop = 1; //1: on , 0: off
	ret = setsockopt(rtp_server_socket,IPPROTO_IP,IP_MULTICAST_LOOP,&loop,sizeof(loop));
	if(ret < 0)
	{
		printf("set multicast loop error \n");
		perror("setsockopt");
	}

	#if 1
	//set multicast address 
	struct ip_mreq mreq;

	//mreq.imr_multiaddr.s_addr = inet_addr(MCAST_ADDR);       
	mreq.imr_multiaddr.s_addr = inet_addr(multicast);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	//add multicast group
	ret = setsockopt(rtp_server_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
	if(ret < 0)
	{
		printf("set multicast error \n");
		perror("setsockopt");
	}
	#endif
#endif

#if 1
	int reuse = 1;
	ret = setsockopt(rtp_server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
	if(ret < 0)
	{
		printf("set multicast reuse error \n");
		perror("setsockopt");
	}
#endif

    struct timeval tv1, tv2, timeout = {2,0};
    //struct timezone tz;
	setsockopt(rtp_server_socket,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
	setsockopt(rtp_server_socket,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));

	unsigned char *buf = (unsigned char*)malloc(sizeof(unsigned char)*BUFFER_SIZE);
	
    unsigned int DataFrameLen,DataFrameLeftLen = 0;
    
    unsigned int iPacketLeftLen =0;
    
    unsigned char uFrameErrorFlag = 0;
    
    unsigned int size = 0;
	
	int bStartRecv=0;
    DATAHEAD *pDataHead = (DATAHEAD *)buf;

	while (1)
	{
Recv:
		#if 1
		//recv data
		len = recvfrom(rtp_server_socket, buf, BUFFER_SIZE, 0, (struct sockaddr_in *) &client_addr, &clielen_addr_length);
		if (len < 0)
		{
			perror("recvfrom");
		}
		else if (len > 0)
		{
			//printf("len : %d \n", len);
			#if 1
			if (0==bStartRecv)
			{
				//check data header
				if (0x1A1B1C1D == pDataHead->iProbe && len>=sizeof(DATAHEAD)) //ignore the DATAHEAD split frame issue
				{
					uNowPayloadType = pDataHead->uPayloadType;
					iPacketLeftLen = len;
#ifdef CHECKSUM_AUDIO
					if (H264 == uNowPayloadType)
					{
						//printf("\n------------------------%d----------------------\n", pDataHead->uSeq);
						//printf("DataFrameLen : %d \n", DataFrameLen);
						usVideoChecksum = pDataHead->usChecksum;
					}
					if (WAV == uNowPayloadType)
					{
						//printf("\n*****************************************\n");
						//printf("DataFrameLen : %d \n", DataFrameLen);
						usAudioChecksum = pDataHead->usChecksum;
					}
#endif
					//printf("iProbe = %0x \n",pDataHead->iProbe);
					//printf("iTimeStamp : %d \n", pDataHead->iTimeStamp);
					//printf("uSeq : %d \n", pDataHead->uSeq);
					//printf("Find probe \n");
					//printf("Payload Type : %d \n", pDataHead->uPayloadType);
					printf("Find len = %d \n",pDataHead->iLen);
					
					bStartRecv = 1;
					ePacketType = START_PACKET;
					uFrameErrorFlag = 0;
					
					//DataFrameLen = pDataHead->iLen;
					//printf("0==bStartRecv \n");
					ret = process_start_frame(buf,len,&size,&DataFrameLeftLen);
					if(-1==ret)
					{
						uFrameErrorFlag = 1;
						bStartRecv = 0;
					}
					else if(0 == ret)
					{
						eNextPacketType = START_PACKET;
					}
					else if(1 ==ret)
					{
						eNextPacketType = CONTINUE_PACKET;
					}
				}
			}
			else //not first frame
			{
				iPacketLeftLen = len;
				//printf("eNextPacketType : %d \n", eNextPacketType);
				//printf("iPacketLeftLen : %d \n", iPacketLeftLen);
				//printf("DataFrameLeftLen : %d \n", DataFrameLeftLen);
				
				if(CONTINUE_PACKET == eNextPacketType)
				{
					//printf("DataFrameLeftLen : %d \n", DataFrameLeftLen);
					//printf("iPacketLeftLen : %d \n", iPacketLeftLen);
					if(DataFrameLeftLen == iPacketLeftLen)  //finish this frame
					{
						//copy frame data to pFrame
						//printf("copy process 1, len = %d \n", DataFrameLeftLen);
						//printf("DataFrameLeftLen == iPacketLeftLen \n");
						memcpy(pFrame+size, buf, DataFrameLeftLen);
						//process pFrame
						size = size + DataFrameLeftLen;
						if (H264 == uNowPayloadType)
						{
							//printf("\n----------------H264 3-------------- \n");
							//printf("Wac process 3, size = %d \n",size);
#ifdef CHECKSUM_VIDEO
							//printf("videochecksum: %x \n", usVideoChecksum);
							//printf("csum(pFrame, size): %x \n", csum((unsigned char *)pFrame, size));
							//printf("*pFrame: %x \n", *pFrame);
							if (size < CHECK_SUM_COUNT)
							{
								if (csum(pFrame, size) == usVideoChecksum)
								{
									wacPushFrameToMDev(pFrame,size);
								}
								else
								{
									printf("video checksum error !\n");
								}
							}
							else
							{
								if (csum(pFrame, CHECK_SUM_COUNT) == usVideoChecksum)
								{
									wacPushFrameToMDev(pFrame,size);
								}
								else
								{
									printf("video checksum error !\n");
								}
							}
#else
							wacPushFrameToMDev(pFrame,size);
#endif
							//fwrite(pFrame, size, 1, outfile);
						}
						if (WAV == uNowPayloadType)
						{
							//printf("\n***************WAV 3*****************\n");
							//printf("size : %d \n", size);
#ifdef CHECKSUM_AUDIO
							//printf("sum: %x \n", csum(pFrame, size));
							//printf("usAudioChecksum : %x \n", usAudioChecksum);
							//printf("pFrame: %d \n", *pFrame);
							#if 0
							int m, n;
							unsigned char * p = pFrame;
							for (m=256; m>0; m--)
							{
								for (n=40; n>0; n--)
								{
									printf("%x",*p);
									p+=1;
								}
								printf("\n");
							}
							#endif
							if (csum(pFrame, size) == usAudioChecksum)
							{
								audio_play(pFrame, size);
							}
							else
							{
								printf("audio checksum error !\n");
							}
							
#else
							audio_play(pFrame, size);
#endif
							//fwrite(pFrame, size, 1, outfile);
						}
						eNextPacketType = START_PACKET;
					}
					else if(DataFrameLeftLen > iPacketLeftLen) //still not finish this frame
					{
						//printf("DataFrameLeftLen > iPacketLeftLen \n");
						//copy frame data to pFrame
						//printf("copy process 2, len = %d \n", iPacketLeftLen);
						memcpy(pFrame+size, buf, iPacketLeftLen);
						size += iPacketLeftLen;
						DataFrameLeftLen -= iPacketLeftLen;
						eNextPacketType = CONTINUE_PACKET;
					}
					else if(DataFrameLeftLen <= iPacketLeftLen - sizeof(DATAHEAD))
					{
						//printf("DataFrameLeftLen <= iPacketLeftLen - sizeof(DATAHEAD)\n");
						//process the frame
						//printf("copy process 2, len = %d \n", iPacketLeftLen);
						memcpy(pFrame+size, buf, DataFrameLeftLen);
						size = size + DataFrameLeftLen;
						if (H264 == uNowPayloadType)
						{
							//printf("\n----------------H264 4-------------- \n");
							//printf("Wac process 4, size = %d \n",size);
							wacPushFrameToMDev(pFrame,size);

						}
						if (WAV == uNowPayloadType)
						{
							//printf("\n***************WAV 4*****************\n");
							//printf("size : %d \n", size);
#ifdef CHECKSUM_AUDIO
							if (csum(pFrame, size) == usAudioChecksum)
							{
								audio_play(pFrame, size);
							}
							else
							{
								printf("audio checksum error !\n");
							}
#else
							audio_play(pFrame, size);
#endif
						}
						ret = process_start_frame( buf+DataFrameLeftLen, iPacketLeftLen-DataFrameLeftLen,&size,&DataFrameLeftLen);
						if(-1==ret)
						{
							uFrameErrorFlag = 1;
							bStartRecv = 0;
						}
						else if(0 == ret)
						{
							eNextPacketType = START_PACKET;
						}
						else if(1 ==ret)
						{
							eNextPacketType = CONTINUE_PACKET;
						}
					}
					else if ((DataFrameLeftLen > (iPacketLeftLen - sizeof(DATAHEAD))) && (DataFrameLeftLen < iPacketLeftLen))
					{
						printf("///////////////////////////// error 1 //////////////////////////////");
						#if 0
						memcpy(pFrame+size, buf, DataFrameLeftLen); 
						size = size + DataFrameLeftLen; //算出这一帧的大小
						
						if (H264 == uNowPayloadType)  //对这一帧数据进行处理
						{
							printf("\n----------------H264 5-------------- \n");
							printf("Wac process 5, size = %d \n",size);
#ifdef CHECKSUM_VIDEO
							if (size < CHECK_SUM_COUNT)
							{
								if (csum(pFrame, size) == usVideoChecksum)
								{
									wacPushFrameToMDev(pFrame,size);
								}
								else
								{
									printf("video checksum error !\n");
								}
							}
							else
							{
								if (csum(pFrame, CHECK_SUM_COUNT) == usVideoChecksum)
								{
									wacPushFrameToMDev(pFrame,size);
								}
								else
								{
									printf("video checksum error !\n");
								}
							}
#else
							wacPushFrameToMDev(pFrame,size);
#endif
							//fwrite(pFrame, size, 1, outfile);
						}
						if (WAV == uNowPayloadType)
						{
							//printf("\n***************WAV 5*****************\n");
							//printf("size : %d \n", size);
#ifdef CHECKSUM_AUDIO
							if (csum(pFrame, size) == usAudioChecksum)
							{
								audio_play(pFrame, size);
							}
							else
							{
								printf("audio checksum error !\n");
							}
#else
							audio_play(pFrame, size);
#endif
							//fwrite(pFrame, size, 1, outfile);
						}
						#endif
						//split DATAHEAD
						printf(" Data Head in two packet, will do it in future \n");
						uFrameErrorFlag = 1;
						bStartRecv =0;
						//break;
					}
				}
				else if(START_PACKET == eNextPacketType)
				{
					//uNowPayloadType = pDataHead->uPayloadType;
					//DataFrameLeftLen = pDataHead->iLen;
					
					//printf("iProbe = %0x \n",pDataHead->iProbe);
					//printf("iTimeStamp : %d \n", pDataHead->iTimeStamp);
					//printf("uSeq : %d \n", pDataHead->uSeq);
					//printf("payload type : %d \n", pDataHead->uPayloadType);
					//printf("Find len = %d \n",pDataHead->iLen);
					//printf("START_PACKET == eNextPacketType \n");
					ret = process_start_frame( buf,len,&size,&DataFrameLeftLen);
					//printf("ret = %d \n", ret);
					if (-1==ret)
					{
						uFrameErrorFlag = 1;
						bStartRecv = 0;
					}
					else if(0 == ret)
					{
						eNextPacketType = START_PACKET;
					}
					else if(1 == ret)
					{
						eNextPacketType = CONTINUE_PACKET;
					}
				}
			}
			#endif
		}
		else
		{
			printf("Recv data len = 0 \n");
			//close(rtp_server_socket);
			//goto ReSocket;
		}
#endif
	
	}
	close(rtp_server_socket);
	free(buf);

	return 0;
}

#else
//TCP
void *app_rtp_main(void)
{
#if 0
    if(!outfile)
	{
		if ((outfile = fopen(outfilename, "wb")) == NULL) 
		{
			printf("can't open %s",outfilename);
			return;
		}
	}
#endif
#if 1
	//set socket's address information
	const int opt = -1;
	int sock_cli;
	socklen_t servlen_addr_length;
    struct sockaddr_in servaddr;
    servlen_addr_length = sizeof(servaddr);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(RTP_PORT);
#ifdef RTP_UDP
	servaddr.sin_addr.s_addr=htonl(INADDR_BROADCAST); //UDP broadcast address 
#else
	servaddr.sin_addr.s_addr = inet_addr("192.168.36.3"); //TCP
#endif

ReSocket:

#ifdef RTP_UDP
    sock_cli = socket(AF_INET,SOCK_DGRAM, 0); //UDP
#else
    sock_cli = socket(AF_INET,SOCK_STREAM, 0); //TCP
#endif
    if (sock_cli < 0)
    {
		perror("socket");
		sleep(1);
		goto ReSocket;
	}
#ifdef RTP_UDP
	setsockopt(sock_cli,SOL_SOCKET,SO_BROADCAST,(char*)&opt,sizeof(opt));
#endif
	struct timeval timeout = {2,0};
	setsockopt(sock_cli,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
	setsockopt(sock_cli,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
ReConnect:
	audio_play(NULL, 0); //audio mute
	//printf("-------------------- Data Channel Connection start----------------------\n");
#ifndef  RTP_UDP

    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("8001 data channel connect error \n");
        perror("connect");
        sleep(1);
        close(sock_cli);
        goto ReSocket;
    }
    printf("8001 data channel Connected \n");

#endif
#endif

	unsigned char *buf = (unsigned char*)malloc(sizeof(unsigned char)*BUFFER_SIZE);
	
    unsigned int DataFrameLen,DataFrameLeftLen = 0;
    
    unsigned int iPacketLeftLen =0;
    
    unsigned char uFrameErrorFlag = 0;
    
    unsigned int size = 0;
    
    int ret;
	int len;

	int bStartRecv=0;
    DATAHEAD *pDataHead = (DATAHEAD *)buf;

	audio_config(48000, 16, 2);

	//packet is TCP packet, frame is 264 frame;
	while(1) //continue get packet
	{
#if 0
		len = sendto(sock_cli, buf, sizeof(DATAHEAD), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
		printf("len : %d \n", len);
#endif
#if 1
		//recv data
#ifdef RTP_UDP
		len = recvfrom(sock_cli, buf, BUFFER_SIZE, 0, (struct sockaddr_in *) &servaddr, &servlen_addr_length);
#else
		len = recv(sock_cli, buf, BUFFER_SIZE, 0);
#endif
		if (len < 0)
		{
			printf("Server Recieve Data Failed!\n");
			close(sock_cli);
			sleep(1);
			goto ReSocket;
		}
		else if (len > 0)
		{
			printf("recv packet len : %d \n", len);
			
			if (0==bStartRecv)
			{
				//check data header
				if(0x1A1B1C1D == pDataHead->iProbe && len>=sizeof(DATAHEAD)) //ignore the DATAHEAD split frame issue
				{
					uNowPayloadType = pDataHead->uPayloadType;
					
					iPacketLeftLen = len;
					
					//printf("iProbe = %0x \n",pDataHead->iProbe);
					//printf("iTimeStamp : %d \n", pDataHead->iTimeStamp);
					//printf("uSeq : %d \n", pDataHead->uSeq);
					//printf("Find probe \n");
					//printf("Payload Type : %d \n", pDataHead->uPayloadType);
					printf("Find len = %d \n",pDataHead->iLen);
					
					bStartRecv = 1;
					ePacketType = START_PACKET;
					uFrameErrorFlag = 0;
					
					//DataFrameLen = pDataHead->iLen;
					//printf("0==bStartRecv \n");
					ret = process_start_frame(buf,len,&size,&DataFrameLeftLen);
					if(-1==ret)
					{
						uFrameErrorFlag = 1;
						bStartRecv = 0;
					}
					else if(0 == ret)
					{
						eNextPacketType = START_PACKET;
					}
					else if(1 ==ret)
					{
						eNextPacketType = CONTINUE_PACKET;
					}
				}
			}
			else//not first frame
			{
				iPacketLeftLen = len;
				if(CONTINUE_PACKET == eNextPacketType)
				{
					if(DataFrameLeftLen == iPacketLeftLen)  //finish this frame
					{
						//copy frame data to pFrame
						//printf("copy process 1, len = %d \n", DataFrameLeftLen);
						memcpy(pFrame+size, buf, DataFrameLeftLen);
						//process pFrame
						size = size + DataFrameLeftLen;
						if (H264 == uNowPayloadType)
						{
							printf("\n----------------H264 3-------------- \n");
							printf("Wac process 3, size = %d \n",size);

							wacPushFrameToMDev(pFrame,size);

						}
						if (WAV == uNowPayloadType)
						{
							printf("\n***************WAV 3*****************\n");
							printf("size : %d \n", size);
							audio_play(pFrame, size);
							//fwrite(pFrame, size, 1, outfile);
						}
						eNextPacketType = START_PACKET;
					}
					else if(DataFrameLeftLen > iPacketLeftLen) //still not finish this frame
					{
						//copy frame data to pFrame
						//printf("copy process 2, len = %d \n", iPacketLeftLen);
						memcpy(pFrame+size, buf, iPacketLeftLen);
						size += iPacketLeftLen;
						DataFrameLeftLen -= iPacketLeftLen;
						eNextPacketType = CONTINUE_PACKET;
					}
					else if(DataFrameLeftLen <= iPacketLeftLen - sizeof(DATAHEAD))
					{
						//process the frame
						//printf("copy process 2, len = %d \n", iPacketLeftLen);
						memcpy(pFrame+size, buf, DataFrameLeftLen);
						size = size + DataFrameLeftLen;
						if (H264 == uNowPayloadType)
						{
							printf("\n----------------H264 4-------------- \n");
							printf("Wac process 4, size = %d \n",size);
							wacPushFrameToMDev(pFrame,size);
						}
						if (WAV == uNowPayloadType)
						{
							printf("\n***************WAV 4*****************\n");
							printf("size : %d \n", size);
							audio_play(pFrame, size);
							//fwrite(pFrame, size, 1, outfile);
						}
						ret = process_start_frame( buf+DataFrameLeftLen, iPacketLeftLen-DataFrameLeftLen,&size,&DataFrameLeftLen);
						if(-1==ret)
						{
							uFrameErrorFlag = 1;
							bStartRecv = 0;
						}
						else if(0 == ret)
						{
							eNextPacketType = START_PACKET;
						}
						else if(1 ==ret)
						{
							eNextPacketType = CONTINUE_PACKET;
						}
					}
					else if ((DataFrameLeftLen > (iPacketLeftLen - sizeof(DATAHEAD))) && (DataFrameLeftLen < iPacketLeftLen))
					{
						printf("/////////////////////////////error 1 //////////////////////////////");
						
						memcpy(pFrame+size, buf, DataFrameLeftLen); 
						size = size + DataFrameLeftLen; //算出这一帧的大小
						
						if (H264 == uNowPayloadType)  //对这一帧数据进行处理
						{
							printf("\n----------------H264 5-------------- \n");
							printf("Wac process 5, size = %d \n",size);
							wacPushFrameToMDev(pFrame,size);
						}
						if (WAV == uNowPayloadType)
						{
							printf("\n***************WAV 5*****************\n");
							printf("size : %d \n", size);
							audio_play(pFrame, size);
							//fwrite(pFrame, size, 1, outfile);
						}
						
						//split DATAHEAD
						printf(" Data Head in two packet, will do it in future \n");
						uFrameErrorFlag = 1;
						bStartRecv =0;
						//break;
					}
				}
				else if(START_PACKET == eNextPacketType)
				{
					//uNowPayloadType = pDataHead->uPayloadType;
					//DataFrameLeftLen = pDataHead->iLen;
					
					//printf("iProbe = %0x \n",pDataHead->iProbe);
					//printf("iTimeStamp : %d \n", pDataHead->iTimeStamp);
					//printf("uSeq : %d \n", pDataHead->uSeq);
					//printf("payload type : %d \n", pDataHead->uPayloadType);
					//printf("Find len = %d \n",pDataHead->iLen);
					//printf("START_PACKET == eNextPacketType \n");
					ret = process_start_frame( buf,len,&size,&DataFrameLeftLen);
					printf("ret = %d \n", ret);
					if (-1==ret)
					{
						uFrameErrorFlag = 1;
						bStartRecv = 0;
					}
					else if(0 == ret)
					{
						eNextPacketType = START_PACKET;
					}
					else if(1 == ret)
					{
						eNextPacketType = CONTINUE_PACKET;
					}
				}
			}
		}
		else
		{
			printf("Recv data len = 0 \n");
			close(sock_cli);
			goto ReSocket;
		}
#endif
	}

    
    close(sock_cli);
	free(buf);

    return 0;
}

#endif




