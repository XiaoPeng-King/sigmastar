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
#include <signal.h>

#include "app_tx_broadcast.h"
#include "sharemem.h"
#include "linked_list.h"

extern unsigned char g_Exit;
#define DEBUG

static unsigned char conflict_buf[128] = {0};

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

static void updateOffLineDeviceNumbers(int sig)
{
	unsigned int i = 0;
	unsigned char tx_online_count = 0, rx_online_count = 0;
#if 1
	if (SIGALRM == sig)
	{
		printf("update off-line device numbers \n");
		//Check off-line RX's devices
		for (i=0; i<255; i++)
		{
			if (1 == share_mem->ucOnlineNum[i])
			{
				if (i <= 128)
				{
					rx_online_count++;
				}
				if (i > 200)
				{
					tx_online_count++;
				}
				
				share_mem->ucOnlineNum[i] = 0;
				
			}
		}

		for (i=0; i<128; i++)
		{
			conflict_buf[i] = 0;
			cleanAllNode(share_mem->sm_group_pack.uuid[i]);
		}
	
		printf("tx on-line number is %d \n", tx_online_count);
		printf("rx on-line number is %d \n", rx_online_count);

		alarm(20);
	}
#endif	
	return ;
}

static unsigned char getConflictNumber(void)
{
	unsigned char i = 0, number = 0;

	for (i=0; i<128; i++)
	{
		if (share_mem->sm_group_pack.uuid[i] != NULL)
		{
			if (share_mem->sm_group_pack.uuid[i]->data > 1)
			{
				conflict_buf[number] = i;
				#ifdef DEBUG
				printf("conflict buf [%d] = %d \n", number, i);
				#endif
				number++;
			}
		}
	}
	#ifdef DEBUG
	printf("conflict number is : %d \n", number);
	#endif
	return number;
}

static int broadcast_send(char command)
{
	int sockfd = -1;
	int ret = -1;
	int len = -1;
	struct sockaddr_in servaddr;
	
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(CLI_UDP_SEND_PORT);
	servaddr.sin_addr.s_addr=htonl(INADDR_BROADCAST); 
	
	int servlen_addr_length = sizeof(servaddr);

	sockfd = CreateUDPSocket();	//socket UDP

	//set socket broadcast
	AllowSocketSendBroadcast(sockfd);
	SetSendTimeOut(sockfd, 2, 0);

#if 1
	if (command == IP_CONFLICT)
	{
		unsigned char i=0, j=0;
		unsigned char number = getConflictNumber();
		unsigned char cmd[number+3];

		cmd[0] = 0xfe;
		cmd[1] = command; //type
		cmd[2] = number;  //data length
		for (i=3; i<number+3; i++)
		{
			cmd[i] = conflict_buf[j];
			j++;
		}

		len = SendToDestAddr(sockfd, &cmd, sizeof(cmd), \
		0, (struct sockaddr *)&servaddr, sizeof(servaddr));
		if (len > 0)
		{
			#ifdef DEBUG
			printf("\nconflict display broadcast sendto success \n");
			#endif
		}
	}
	if (command == INFO_DISPLAY)
	{
		unsigned char cmd[4];
		cmd[0] = 0xfe;
		cmd[1] = command;
		cmd[2] = 1;
		cmd[3] = 1; //ON info display

		len = SendToDestAddr(sockfd, &cmd, sizeof(cmd), \
		0, (struct sockaddr *)&servaddr, sizeof(servaddr));
		if (len > 0)
		{
			#ifdef DEBUG
			printf("\ninfo display broadcast sendto success \n");
			#endif
		}
	}

#endif
	close(sockfd);
	return 0;
}

void *control_slave()
{
	int sockfd = -1;
	int ret = -1;
	int len = -1;
	struct sockaddr_in servaddr;
	
	const int opt = -1;
	
	REPORT_PACK_S broadCollect_s;
	REPORT_PACK_S broadSend_s;
	
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_UDP_PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr("0.0.0.0");
	//servaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST); //broadcast address
	
	int servlen_addr_length = sizeof(servaddr);

	sockfd = CreateUDPSocket();	//socket UDP
	//set socket broadcast
	AllowSocketSendBroadcast(sockfd);

	SocketBindToAddr(sockfd, (struct sockaddr_in *) &servaddr, sizeof(servaddr)); //bind
	
	SetRecvTimeOut(2, 0);
	SetSendTimeOut(2, 0);
	
	signal(SIGALRM, updateOffLineDeviceNumbers);
	alarm(20);
	unsigned char RX_ID;

	while (g_Exit)
	{
		//sleep(1);
		if (readable_timeo(sockfd, 2))
		{
			len = RecvFromSrcAddr(sockfd, &broadCollect_s, sizeof(broadCollect_s), \
				0, (struct sockaddr_in *) &servaddr, &servlen_addr_length);
			if (len > 0)
			{
				#ifdef DEBUG
				printf("\n-------------------- recv successful ------------------------\n");
				printf("recv len = %d \n", len);
				printf("recive Ip : %d, uuid : %d \n", broadCollect_s.ucIpAddress, broadCollect_s.uuid);
				printf("broadCollect_s.ucMultiAddress : %d \n", broadCollect_s.ucMultiAddress);
				printf("broadCollect_s.uProbe : %d \n", broadCollect_s.uProbe);
				printf("broadSend_s.ucInfoDisplayFlag : %d \n", broadSend_s.ucInfoDisplayFlag);
				#endif
				if (PROBE == broadCollect_s.uProbe) //probe
				{
					if (1 == share_mem->ucInfoDisplayFlag)
					{
						broadSend_s.ucInfoDisplayFlag = 1;
						broadcast_send(INFO_DISPLAY);
						continue;
					}
					else
					{
						broadSend_s.ucInfoDisplayFlag = 0;
					}
					if (broadCollect_s.ucIpAddress <= 128) //ip
					{
						RX_ID = broadCollect_s.ucIpAddress;
						broadSend_s.ucIpAddress = broadCollect_s.ucIpAddress;
						broadSend_s.ucMultiAddress = broadCollect_s.ucMultiAddress;
						broadSend_s.uProbe = broadCollect_s.uProbe;
						broadSend_s.uuid = broadCollect_s.uuid;
						share_mem->ucOnlineNum[broadCollect_s.ucIpAddress] = 1;
					}
					else if ((broadCollect_s.ucIpAddress) > 200 && (broadCollect_s.ucIpAddress < 256)) //TX
					{
						share_mem->ucOnlineNum[broadCollect_s.ucIpAddress] = 1;
						//printf("This is TX device \n");
						continue;
					}
					else
					{
						printf("Slave ip address is error\n");
						continue;
					}
				}
				
				if (NULL == share_mem->sm_group_pack.uuid[RX_ID]) //
				{
					//printf("Create a new node : %d  \n", RX_ID);
					share_mem->sm_group_pack.uuid[RX_ID] = createHeadNode();
					//printf("head node data : %d \n", share_mem->sm_group_pack.uuid[RX_ID]->data);
				}

				if(1 != (ifSameData(share_mem->sm_group_pack.uuid[RX_ID], broadCollect_s.uuid)))
				{
					//printf("uuid: %d \n", broadCollect_s.uuid);
					if (0 != broadCollect_s.uuid)
					{
						insertNewNodeByBack(share_mem->sm_group_pack.uuid[RX_ID], broadCollect_s.uuid);
						//printf("insert RX : %d , uuid : %d \n", RX_ID, broadCollect_s.uuid);
					}
					//printf("share_mem->sm_group_pack.uuid[%d]->data : %d \n", RX_ID, share_mem->sm_group_pack.uuid[RX_ID]->data);
				}
				broadSend_s.ucIpRepeat = findAllNodeNumber(share_mem->sm_group_pack.uuid[RX_ID]);
				if (broadSend_s.ucIpRepeat >= 2)
				{
					#ifdef DEBUG
					printf("RX[%d] number are %d, please check \n", RX_ID, broadSend_s.ucIpRepeat);
					int i;

					for (i=1; i<=broadSend_s.ucIpRepeat; i++)
					{
						printf("RX %d - uuid %d : %d \n", RX_ID, i, findNodeData(share_mem->sm_group_pack.uuid[RX_ID], i));
					}
					#endif

					broadcast_send(IP_CONFLICT);
				}
				else
				{
					if (1 == share_mem->ucControlBoxFlag)
					{
						printf("share_mem->sm_group_pack.ucMultiAddress[%d] : %d\n", broadSend_s.ucIpAddress, share_mem->sm_group_pack.ucMultiAddress[broadSend_s.ucIpAddress]);
						printf("broadSend_s.ucMultiAddress : %d\n", broadSend_s.ucMultiAddress);
						if (share_mem->sm_group_pack.ucMultiAddress[broadSend_s.ucIpAddress] == broadSend_s.ucMultiAddress)
						{
							printf("Multicast is the same \n");
							continue;
						}
						else
						{
							broadSend_s.ucMultiAddress = share_mem->sm_group_pack.ucMultiAddress[broadSend_s.ucIpAddress];
						}
					}
					else
					{
						printf("Control Box disable \n");
						continue;
					}
				}
#if 1
				len = SendToDestAddr(sockfd, &broadSend_s, sizeof(broadSend_s), \
					0, (struct sockaddr *)&servaddr, sizeof(servaddr));
				if (len > 0)
				{
					#ifdef DEBUG
					printf("----------------- sendto success ------------------------\n");
					printf("len = %d \n", len);
					printf("broadSend_s.ucIpAddress: %d \n", broadSend_s.ucIpAddress);
					printf("broadSend_s.ucMultiAddress: %d \n", broadSend_s.ucMultiAddress);
					printf("broadSend_s.uuid: %d \n", broadSend_s.uuid);
					printf("broadSend_s.ucIpRepeat: %d \n", broadSend_s.ucIpRepeat);
					#endif
				}
#endif
			}
		}
	}
	printf("------control slave thread exit--------\n");
	close(sockfd);
	return 0;
}




