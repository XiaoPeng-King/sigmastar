#ifndef __APP_TX_BROADCAST_H_
#define __APP_TX_BROADCAST_H_

#define SERV_UDP_PORT	8888
#define CLI_UDP_SEND_PORT	8889
#define PROBE		0xFEDCBA
#define IP_CONFLICT 0x02
#define INFO_DISPLAY 0x03
#define ON	1
#define OFF 0
#define ALL 0

typedef struct
{
	unsigned char ucIpAddress;
	unsigned char ucMultiAddress;
	unsigned int uProbe;
	unsigned int uuid;
	unsigned char ucInfoDisplayFlag;
	unsigned char ucIpRepeat;
	
} REPORT_PACK_S;

void *control_slave();

#endif
