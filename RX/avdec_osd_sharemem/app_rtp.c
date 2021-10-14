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
#include "linux_common.h"
#include "crc.h"
#include "sharemem.h"

//#define RTP_UDP
#define CHECKSUM_AUDIO
#define CHECKSUM_VIDEO
//#define DEBUG

static FILE * outfile  = NULL;

static unsigned char uNowPayloadType;

#define MULTICAST_LEN	20
static char multicast_tmp[MULTICAST_LEN];

char idr_flag = 0;
char outfilename[128] = "./test2.264";
FILE *p264File;

unsigned short int usAudioChecksum;
unsigned short int usVideoChecksum;

#if 1
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
				printf("---audio checksum error !\n");
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
				printf("----audio checksum error !\n");
			}
#else
			audio_play(pFrame, size);
#endif
			//fwrite(pFrame, size, 1, outfile);
		}
		
		return 0;
	}
}

//UDP
void *app_rtp_main(void *args)
{
	int ret, len, timeOut = 0;
	//p264File = fopen("./test.264", "w+");

#if 1
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
	rtp_server_socket = CreateUDPSocket(); //UDP
	
    //bind
    SocketBindToAddr(rtp_server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
#endif

#if 1
	//set multicast loop
	SetMulticastLoop(rtp_server_socket);
	//set socket to add multicast address
	//AddSocketToMulticast(rtp_server_socket, g_multicast);
	AddSocketToMulticast(rtp_server_socket, share_mem->sm_eth_setting.strEthMulticast);

	//set socket reuse the addresss 
	SetReuseSocketAddr(rtp_server_socket);
	//set socket recive timeout
	SetRecvTimeOut(rtp_server_socket, 2, 0);
#endif

	unsigned char *buf = (unsigned char*)malloc(sizeof(unsigned char)*BUFFER_SIZE);
    unsigned int DataFrameLen,DataFrameLeftLen = 0;
    unsigned int iPacketLeftLen =0;
    unsigned char uFrameErrorFlag = 0;
    unsigned int size = 0;
	int bStartRecv=0;
    DATAHEAD *pDataHead = (DATAHEAD *)buf;

	while (1)
	{
		if (0 != memcmp(multicast_tmp, share_mem->sm_eth_setting.strEthMulticast, MULTICAST_LEN))
		{
			memcpy(multicast_tmp, share_mem->sm_eth_setting.strEthMulticast, MULTICAST_LEN);
			close(rtp_server_socket);
			goto ReSocket;
		}
Recv:
		#if 1
		//recv data
		len = RecvFromSrcAddr(rtp_server_socket, buf, BUFFER_SIZE, 0, (struct sockaddr_in *) &client_addr, &clielen_addr_length);
		if (len > 0)
		{
			#ifdef DEBUG
			//printf("len : %d \n", len);
			#endif
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
						#ifdef DEBUG
						printf("\n------------------------%d----------------------\n", pDataHead->uSeq);
						printf("DataFrameLen : %d \n", DataFrameLen);
						#endif
						usVideoChecksum = pDataHead->usChecksum;
					}
					if (WAV == uNowPayloadType)
					{
						//printf("\n*****************************************\n");
						//printf("DataFrameLen : %d \n", DataFrameLen);
						usAudioChecksum = pDataHead->usChecksum;
					}
#endif
					#ifdef DEBUG
					printf("iProbe = %0x \n",pDataHead->iProbe);
					printf("iTimeStamp : %d \n", pDataHead->iTimeStamp);
					printf("uSeq : %d \n", pDataHead->uSeq);
					printf("Find probe \n");
					printf("Payload Type : %d \n", pDataHead->uPayloadType);
					printf("Find len = %d \n",pDataHead->iLen);
					#endif
					
					bStartRecv = 1;
					ePacketType = START_PACKET;
					uFrameErrorFlag = 0;
					
					//DataFrameLen = pDataHead->iLen;
					//printf("0==bStartRecv \n");
					ret = process_start_frame(buf,len,&size,&DataFrameLeftLen);
					if(-1 == ret)
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
								printf("-audio checksum error !\n");
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
								printf("--audio checksum error !\n");
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
						printf("///////////////////////////// error 1 //////////////////////////////\n");
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
#endif
	}
	close(rtp_server_socket);
	free(buf);

	return 0;
}

