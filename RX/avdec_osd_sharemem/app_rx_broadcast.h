#ifndef __APP_RX_BROADCAST_H_
#define __APP_RX_BROADCAST_H_

#define CLI_UDP_PORT	8888
#define CLI_UDP_RECIVE_PORT	8889
#define PROBE		0xFEDCBA
#define ON	1
#define OFF 0


typedef struct
{
	unsigned char ucIpAddress;
	unsigned char ucMultiAddress;
	unsigned int uProbe;
	unsigned int uuid;
	unsigned char ucInfoDisplayFlag;
	unsigned char ucIpRepeat;

} REPORT_PACK_S;


void *IP_broadcast_report();
void *IP_broadcast_recive();



#endif
