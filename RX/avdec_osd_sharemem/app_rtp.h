#ifndef _APP_RTP_H_
#define _APP_RTP_H_


#define RTP_PORT    8001
#define LENGTH_OF_LISTEN_QUEUE     20
#define BUFFER_SIZE                2*1024*1024 //1M
#define DATANUM 10000000

#define MCAST_ADDR		"239.255.42.44"

#define H264	0
#define WAV		1

#define CECKHEAD	0x1A1B1C1D

#define CHECK_SUM_COUNT 1024*50

typedef struct
{
    unsigned int iProbe;
    unsigned char uPayloadType;
	unsigned char uSeq:8;
	unsigned int  iLen:24;
	unsigned int iTimeStamp;
	unsigned short int usChecksum :16;
}DATAHEAD;

enum PACKETTYPE
{
	START_PACKET,
	CONTINUE_PACKET
}ePacketType, eNextPacketType;

#if 0
typedef struct   
{
    /**//* byte 0 */  
    unsigned char csrc_len:4;        /**//* expect 0 */  
    unsigned char extension:1;        /**//* expect 1, see RTP_OP below */  
    unsigned char padding:1;        /**//* expect 0 */  
    unsigned char version:2;        /**//* expect 2 */  
    /**//* byte 1 */  
    unsigned char payload:7;        /**//* RTP_PAYLOAD_RTSP */  
    unsigned char marker:1;        /**//* expect 1 */  
    /**//* bytes 2, 3 */  
    unsigned short seq_no;              
    /**//* bytes 4-7 */  
    unsigned  long timestamp;          
    /**//* bytes 8-11 */  
    unsigned long ssrc;            /**//* stream number is used here. */  
} RTP_FIXED_HEADER; 
#endif


/**							RTP FIXED HEADER
 *0+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++32bit
 * |V=2|P|X|	CC	|M|		PT		|		sequence number				|
 * |						timesstamp									|
 * 
 * 
 * 
 * 
 * 
 */

/*
enum PAYLOADTYPE
{
	//H264 = 96,
	
	
}ePayloadType;
*/
#if 0
typedef struct
{
    /**//* byte 0 */
    unsigned char version:2;	//2bit, 
    unsigned char padding:1;	//1bit
    unsigned char extension:1;	//1bit
    unsigned char csrc_len:4;	//4bit

    /**//* byte 1 */
    unsigned char marker:1;
    unsigned char payload_type:7;
   
    /**//* bytes 2, 3 */ 
    unsigned short seq_num;	//16bit
    /**//* bytes 4-7 */  
    unsigned  long timestamp;	//32bit
    /**//* bytes 8-11 */
    unsigned long ssrc;  /* stream number is used here. */  
} RTP_FIXED_HEADER; 
#endif

unsigned char pFrame[BUFFER_SIZE];

void *app_rtp_main(void *args);

#endif
