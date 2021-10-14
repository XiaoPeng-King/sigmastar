#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "linux_common.h"
#include "app_igmp.h"
#include "sharemem.h"

//extern char g_multicast[20];
//extern char web_flag;
//extern char g_multicastChange_flag;
#define MULTICAST_LEN	20
static char multicast_tmp[MULTICAST_LEN];

static unsigned short csum(unsigned short *buf, int nwords)
{
	unsigned long sum;
	
	//printf("buf : 0x%x \n", buf);
	
	for (sum = 0; nwords > 0; nwords--)
	{
		sum += *buf++;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	//printf("buf.usChecksum: 0x%x \n", ~sum);
	return (unsigned short)(~sum);
}

int IGMP_config(const char type)
{	
	if (IGMP_LEAVE==type)
	{
		IGMP_Packet.ucType = type; //report packet
		IGMP_Packet.ucMaxRspCode = 0; //tmieout
		IGMP_Packet.uiAddGroup = inet_addr(multicast_tmp);; //leave address
		IGMP_Packet.usChecksum = 0;
		IGMP_Packet.usChecksum = csum(&IGMP_Packet, sizeof(IGMP_V2));
		//printf("usChecksum : 0x%x \n", IGMP_Packet.usChecksum);
	}
	else
	{
		IGMP_Packet.ucType = type; //report packet
		IGMP_Packet.ucMaxRspCode = 0; //tmieout
		//IGMP_Packet.uiAddGroup = inet_addr(g_multicast); //report address
		IGMP_Packet.uiAddGroup = inet_addr(share_mem->sm_eth_setting.strEthMulticast); //report address
		IGMP_Packet.usChecksum = 0;
		IGMP_Packet.usChecksum = csum(&IGMP_Packet, sizeof(IGMP_V2));
		//printf("usChecksum : 0x%x \n", IGMP_Packet.usChecksum);
		memcpy(multicast_tmp, share_mem->sm_eth_setting.strEthMulticast, MULTICAST_LEN);
	}
}

void *app_igmp_report()
{
	//printf("igmp : %d \n", sizeof(IGMP_V2));
	//printf("unsigned long int : %ld \n", sizeof(unsigned long int));
	int len, ret;
	
#if 1
	//set socket's address information
	int sock_cli;
	socklen_t servlen_addr_length;
    struct sockaddr_in server_addr;
    servlen_addr_length = sizeof(server_addr);
    memset(&server_addr, 0, sizeof(server_addr));

ReSocket:
    //web_flag = 0;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(IGMP_PORT);
	server_addr.sin_addr.s_addr=inet_addr(share_mem->sm_eth_setting.strEthMulticast); //UDP multicast address
	
    //sock_cli = socket(AF_INET,SOCK_DGRAM, 0); //UDP
    sock_cli = CreateIGMPSocket(); //IGMP

	SetRecvTimeOut(sock_cli, 2, 0);
	SetSendTimeOut(sock_cli, 2, 0);

#endif
	IGMP_config(IGMP_REPORT);
	SendToDestAddr(sock_cli, &IGMP_Packet, sizeof(IGMP_V2), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
	//unsigned long int test = 0x17020802e00000fb;
	//unsigned long int test = 0xfb0000e002080217;
	while (1)
	{
		//IGMP_config(IGMP_REPORT);
		//printf("web_flag = %d \n", web_flag);
		if (0 != memcmp(multicast_tmp, share_mem->sm_eth_setting.strEthMulticast, MULTICAST_LEN))
		{
			server_addr.sin_addr.s_addr=inet_addr(LEAVE_ADDR);
			IGMP_config(IGMP_LEAVE);
			SendToDestAddr(sock_cli, &IGMP_Packet, sizeof(IGMP_V2), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
			close(sock_cli);
			goto ReSocket;
		}
		
		SendToDestAddr(sock_cli, &IGMP_Packet, sizeof(IGMP_V2), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
		//SendToDestAddr(sock_cli, &test, sizeof(test), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
		
		//printf("send igmp len : %d \n", len);
		usleep(500000); //0.5s
	}
}


