#ifndef _APP_RTP_TX_H_
#define _APP_RTP_TX_H_

#define RTP_PORT    8001
#define LENGTH_OF_LISTEN_QUEUE     20
#define BUFFER_SIZE               2* 1024*1024 //1M
#define DATANUM 10000000
//#define UDP_MTU	15000//1024*10
#define UDP_MTU	30000//1024*3


#define FETCH_COUNT 1024*6
//#define FETCH_COUNT 1024*15
#define CHECK_SUM_COUNT 1024*50

#define MCAST_ADDR		"239.255.42.44"

#define H264	0
#define WAV		1
#define H264I 	2
#define H264P	3

typedef struct
{
    unsigned int iProbe;
    unsigned char uPayloadType;
	unsigned char uSeq:8;
	unsigned int  iLen:24;
	unsigned int iTimeStamp;
	unsigned short int usChecksum :16;
}DATAHEAD;

void *send_rtp_tx(void);
void *send_tcp_tx(void);

#endif




















