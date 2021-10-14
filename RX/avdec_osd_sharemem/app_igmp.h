#ifndef _APP_IGMP_H_
#define _APP_IGMP_H_


#define IGMP_PORT    8002
#define LEAVE_ADDR		"224.0.0.2"
#define IGMP_ADDR		"224.0.0.251"

#define IGMP_REPORT		0x16
#define IGMP_LEAVE 		0x17

//
typedef struct 
{
	unsigned char ucVersion :4;
	unsigned char ucType :4;
	unsigned char ucReserved :8;
	unsigned short int usChecksum :16;
	unsigned int uiAddGroup :32;
}IGMP_V1;

typedef struct 
{
	unsigned char ucType :8;
	unsigned char ucMaxRspCode :8;
	unsigned short int usChecksum :16;
	unsigned int uiAddGroup :32;
}IGMP_V2;

IGMP_V2 IGMP_Packet, IGMP_Packet_report, IGMP_Packet_leave;


void *app_igmp_report();



#endif



