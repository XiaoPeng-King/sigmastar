#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <netinet/in.h>                         // 包含AF_INET相关结构
#include <arpa/inet.h>                      // 包含AF_INET相关操作的函数
#include <fcntl.h>

#include "app_rx_broadcast.h"
#include "sharemem.h"
#include "init_net.h"
#include "uart_cec.h"
#include "main.h"

extern SYSTEM_ATTR_s g_system_attr;

#define DEBUG
#define CEC_ENABLE

#if 1
static int get_random(void)
{
	struct timeval tv;
    gettimeofday(&tv,NULL);
	
	//printf("(unsigned int)time(NULL) : %d \n", (unsigned int)time(NULL));
	srand((unsigned int)(tv.tv_sec*1000000 + tv.tv_usec));
	
	return rand(); 
}
#endif


#if 1

static int readable_timeo(int fd, int sec)
{
	fd_set rset;
	struct timeval tv;
	
	FD_ZERO(&rset);
	FD_SET(fd, &rset);
	
	tv.tv_sec = sec;
	tv.tv_usec = 0;
	
	return (select(fd+1, &rset, NULL, NULL, &tv));
}
#endif

void *IP_broadcast_report()
{
	int sockfd = -1;
	int display_count = 0;
	int ret = -1;
	int len = -1;
	char str_tmp[50];

	unsigned int ip_add, mul_add;
	//int random_number = get_random();
	int random_number = share_mem->uuid;
	socklen_t servlen_addr_length;
	struct sockaddr_in servaddr;
	char * s = malloc(20*sizeof(char));
	REPORT_PACK_S broadReport_s;
	REPORT_PACK_S broadRecv_s;
	
	servlen_addr_length = sizeof(servaddr);
	memset(&servaddr, 0, sizeof(servaddr));
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(CLI_UDP_PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST); //broadcast address
	
try_socket:

	printf("IP_broadcast_report create socked \n");
	sockfd = CreateUDPSocket();	//socket UDP

	//set socket broadcast 
	AllowSocketSendBroadcast(sockfd); 
	
	broadReport_s.uProbe = PROBE;
	broadReport_s.uuid = random_number; //random number

	SetRecvTimeOut(sockfd, 2, 0);
	SetSendTimeOut(sockfd, 2, 0);

	while (1)
	{
		sleep(1);
		
		//get multicast address
		//printf("share_men : %s \n", share_mem->sm_eth_setting.strEthMulticast);
		inet_pton(AF_INET, share_mem->sm_eth_setting.strEthMulticast, &mul_add);
		//printf("mul_add : %d \n", mul_add);
		mul_add = ntohl(mul_add); //host
		mul_add &= 0xFF;
		broadReport_s.ucMultiAddress = mul_add; //multicast address
		//printf("multicast address : %d \n", mul_add);
		
		//get ip address
		inet_pton(AF_INET, share_mem->sm_eth_setting.strEthIp, &ip_add);
		ip_add = ntohl(ip_add); //host
		ip_add &= 0xFF;
		broadReport_s.ucIpAddress = ip_add-1; //ip address, ip address is 1-128 but ucIpAddress is 0-127.
		#ifdef DEBUG
		printf("ip address : %d \n", ip_add);
		printf("broadReport_s.ucIpAddress : %d \n", broadReport_s.ucIpAddress);
		printf("broadReport_s.ucMultiAddress : %d \n", broadReport_s.ucMultiAddress);
		//printf("buffer 1 : %d \n", buffer_flag[1]);
		#endif 
		len = SendToDestAddr(sockfd, &broadReport_s, sizeof(broadReport_s), \
				0, (struct sockaddr *)&servaddr, sizeof(servaddr));
		if (len > 0)
		{
			#ifdef DEBUG
			printf("---------------------- sendto success --------------------\n");
			#endif
			if (readable_timeo(sockfd, 2))
			{
				len = RecvFromSrcAddr(sockfd, &broadRecv_s, sizeof(broadRecv_s), \
					0, (struct sockaddr_in *) &servaddr, &servlen_addr_length);
				if (len > 0)
				{
					#ifdef DEBUG
					printf("--------------------- recv success ------------------\n");
					printf("ucIpAddress : %d \n", broadRecv_s.ucIpAddress);
					printf("ucMultiAddress : %d \n", broadRecv_s.ucMultiAddress);
					printf("uuid : %d \n", broadRecv_s.uuid);
					//printf("ucIpRepeat : %d \n", broadRecv_s.ucIpRepeat);
					#endif
					if (PROBE != broadRecv_s.uProbe)
					{
						printf("probe error \n");
						continue;
					}
					else
					{
						g_system_attr.info_display = broadRecv_s.ucInfoDisplayFlag;
						//printf("ucInfoDisplayFlag: %d \n", broadRecv_s.ucInfoDisplayFlag);
					}

					if (broadReport_s.ucIpAddress != broadRecv_s.ucIpAddress)
					{
						#ifdef DEBUG
						printf("IP address is not same \n");
						#endif
						continue;
					}

					if (broadReport_s.uuid != broadRecv_s.uuid)
					{
						printf("uuid is error \n");
						continue;
					}
					
					if (broadRecv_s.ucIpRepeat > 1)
					{
						//printf("Device ID repeated, please check device switch key ! \n");
						g_system_attr.ip_conflict_flag = 1;
						continue;
					}

					if (broadReport_s.ucMultiAddress == broadRecv_s.ucMultiAddress)
					{
						#ifdef DEBUG
						printf("Multicast address is same \n");
						sprintf(s, "239.255.42.%d", broadRecv_s.ucMultiAddress);
						printf(s);
						#endif
						continue;
					}
					else //change multicast address
					{
						printf("switch rx multicast address \n");
						sprintf(s, "239.255.42.%d", broadRecv_s.ucMultiAddress);
						printf(s);
						strcpy(share_mem->sm_eth_setting.strEthMulticast, s);
						share_mem->ucUpdateFlag = 1;
					}
				}
			}
		}
	}
	
	close(sockfd);
	
	return 0;
}

#if 1
void *IP_broadcast_recive()
{
	int sockfd = -1;
	int display_count = 0;
	int ret = -1;
	int len = -1;
	int i = 0;
	int num = 0;

	unsigned char recive_cmd[128] = {0};
	/*
		recive_cmd[0] : 0xfe
		recive_cmd[1] : command type 
							0x01 : switch rx
							0x02 : ip display
							0x03 : ip and multicast display
							0x04 : off CEC
							0x05 : on CEC
		recive_cmd[2] : data length
		recive_cmd[3] : data 1
		revive_cmd[4] : data 2
	*/
	unsigned char respond_cmd[6];
	/*
		respond_cmd[0] : 0xfe
		respond_cmd[1] : command type
							0xf1 : 
		respond_cmd[2] : data length 
		respond_cmd[3] : Rx number
							1-128
		respone_cmd[4] : Tx number
							1-24
		respone_cmd[5] : switch state:
							0 : switch fail
							1 : switch success
	*/
	//printf("\n---------------------------------------------------------------------\n");
	const int opt = -1;
	unsigned int ip_add, mul_add;
	socklen_t servlen_addr_length;
	struct sockaddr_in servaddr;
	char * s = malloc(20*sizeof(char));
	//REPORT_PACK_S broadReport_s;
	//REPORT_PACK_S broadRecv_s;
	
	servlen_addr_length = sizeof(servaddr);
	memset(&servaddr, 0, sizeof(servaddr));
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(CLI_UDP_RECIVE_PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
try_socket:

	printf("IP_broadcast_recive create socked \n");
	sockfd = CreateUDPSocket();	//socket UDP

	//socket bind to server address
	SocketBindToAddr(sockfd, (struct sockaddr_in *) &servaddr, sizeof(servaddr)); //bind
	//set socket broadcast 
	AllowSocketSendBroadcast(sockfd); 
	
	while (1)
	{	
		//get multicast address
		inet_pton(AF_INET, share_mem->sm_eth_setting.strEthMulticast, &mul_add);
		mul_add = ntohl(mul_add); //host
		mul_add &= 0xFF;
		respond_cmd[3] = mul_add; //multicast address
		//printf("multicast address : %d \n", mul_add);
		
		//get ip address
		inet_pton(AF_INET, share_mem->sm_eth_setting.strEthIp, &ip_add);
		ip_add = ntohl(ip_add); //host
		ip_add &= 0xFF;
		respond_cmd[2] = ip_add; //ip address, ip address is 1-128 but ucIpAddress is 0-127.
		//printf("ip address : %d \n", ip_add);
		{
			if (readable_timeo(sockfd, 3))
			{
				len = RecvFromSrcAddr(sockfd, &recive_cmd, sizeof(recive_cmd), \
				0, (struct sockaddr_in *) &servaddr, &servlen_addr_length);
				if (len > 0)
				{
					printf(" recive_cmd : %s ,and len : %d\n\n",recive_cmd,len);

					#ifdef DEBUG
					printf("cmd[0]: 0x%x \n", recive_cmd[0]); //data head 0xfe
					printf("cmd[1]: %d \n", recive_cmd[1]); //data type
					printf("cmd[2]: %d \n", recive_cmd[2]); //data length 
					printf("cmd[3]: %d \n", recive_cmd[3]); //
					printf("cmd[4]: %d \n", recive_cmd[4]); //
					printf("cmd[5]: %d \n", recive_cmd[5]); //
					#endif

					if (recive_cmd[0] != 0xfe)
					{
						continue;
					}
					switch (recive_cmd[1])
					{
						case 0x01: //switch RX multicast
							printf("switch RX multicast\n");
							if (recive_cmd[2] < 2) //data length
							{
								printf("command type error \n");
								break;
							}
							#if 0
							else if (0 == recive_cmd[3]) //Control HPD of HDMI via command
							{
								if (0 == recive_cmd[4])
								{
									printf("HPD OFF\n");
									OFF_HPD();
									break;
								}
								if (1 == recive_cmd[4])
								{
									printf("HPD ON\n");
									ON_HPD();
									break;
								}
							}
							#endif
							#ifdef CEC_ENABLE
							else if (0 == recive_cmd[3]) //Control HPD of HDMI via command
							{
								if (0 == recive_cmd[4])
								{
									UART_CEC_TV_OFF();
									break;
								}
								if (1 == recive_cmd[4])
								{
									UART_CEC_TV_ON();
									break;
								}
							}
							#endif
							else if (ip_add != recive_cmd[3]) //RX address
							{
								break;
							}
							else if ((recive_cmd[4] > 24) || (recive_cmd[4] == mul_add)) //TX address
							{
								printf("Multicast address is same \n");
								sprintf(s, "239.255.42.%d", recive_cmd[4]);
								printf(s);
								break;
							}
							else //change multicast address
							{
								sprintf(s, "239.255.42.%d", recive_cmd[4]);
								printf(s);
								strcpy(share_mem->sm_eth_setting.strEthMulticast, s);

								share_mem->ucUpdateFlag = 1;
								//printf("-------------------------------------------------------\n");
								//printf("*******************************************************\n");
								//process_osd_text_solid(10, 10, share_mem->sm_eth_setting.strEthMulticast);
								respond_cmd[0] = 0xfe;
								respond_cmd[1] = 0xf1;
								respond_cmd[2] = 3;
								respond_cmd[3] = recive_cmd[3];
								respond_cmd[4] = recive_cmd[4];
								respond_cmd[5] = 1;
								//printf("cmd[0] = %d \n", respond_cmd[0]);
								//printf("cmd[1] = %d \n", respond_cmd[1]);
								//printf("cmd[2] = %d \n", respond_cmd[2]);
								//printf("cmd[3] = %d \n", respond_cmd[3]);
								//printf("cmd[4] = %d \n", respond_cmd[4]);


								len = SendToDestAddr(sockfd, &respond_cmd, sizeof(respond_cmd), \
									0, (struct sockaddr *)&servaddr, sizeof(servaddr));
								if (len > 0)
								{
									break;
								}
							}
							break;

						case 0x02: //display ip conflict
							printf("ip conflict\n");
							for (i=0; i<recive_cmd[2]; i++)
							{
								if (recive_cmd[i+3] == ip_add-1)
								{
									//printf("\n\n-----------------\n\n");
									if (g_system_attr.display_state.signal_state == EN_NORMAL)
										g_system_attr.display_state.signal_normal = EN_IP_CONFLICT;
									break;
								}
							}
							break;

						case 0x03: //display ip and multicast info
							printf("display ip and multicast info \n");
							if (g_system_attr.display_state.signal_state == EN_NORMAL)
								g_system_attr.display_state.signal_normal = EN_INFO_DIAPLAY;
							break;
#ifdef CEC_ENABLE
						case 0x04:
							num = recive_cmd[2];
							
							for (i=0; i<num; i++)
							{
								//printf("ip = %d , recive_cmd[%d] : %d \n", ip_add, i+3, recive_cmd[i+3]);
								if (recive_cmd[i+3] == ip_add)
								{
									printf("CEC OFF \n");
									UART_CEC_TV_OFF();
									break;
								}
							}
							break;

						case 0x05:
							num = recive_cmd[2];

							for (i=0; i<num; i++)
							{
								if (recive_cmd[i+3] == ip_add)
								{
									printf("CEC ON \n");
									UART_CEC_TV_ON();
									break;
								}
							}
							break;
#endif
						default:
							break;
					}
				}
			}
			else
			{
				printf("\n\n------------------ close display --------------\n\n");
				g_system_attr.display_state.signal_normal = EN_CLOSE_DISPLAY;
				
				g_system_attr.display_flag = 0;
				g_system_attr.ip_conflict_flag = 0;
			}
		}
	}
	close(sockfd);
	
	return 0;
}
#endif
