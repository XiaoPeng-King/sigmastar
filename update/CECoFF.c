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
#include <string.h>

#define SERV_UDP_PORT	8889


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


int main(char argc, char *argv[])
{
	int num = 0;
	if (argc < 2)
	{
		printf("command error \n");
		return 0;
	}
	else
	{
		printf("argc : %d \n", argc);
		num = argc - 1;
	}
	
	unsigned char cmd[128] = {0};
	int sockfd = -1;
	int ret = -1;
	int len = -1;
	struct sockaddr_in servaddr;
	
	const int opt = 1;
	
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_UDP_PORT);
	//servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr("0.0.0.0"); 
	//servaddr.sin_addr.s_addr=htonl(INADDR_BROADCAST); //inet_addr("255.255.255.255");
	servaddr.sin_addr.s_addr=inet_addr("192.168.36.255");
	
	int servlen_addr_length = sizeof(servaddr);
try_socket:
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)	//socket UDP
	{
		perror("socket");
		printf("server broadcast socket error \n");
		
		goto try_socket;
	}
	
	//printf("sockfd = %d \n", sockfd);
	
	//set socket broadcast
	ret = setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,(char*)&opt,sizeof(opt));
	#if 0
	if (ret = bind(sockfd, (struct sockaddr_in *) &servaddr, sizeof(servaddr)) < 0) //bind
	{
		perror("bind");
		printf("server broadcast bind error \n");
		close(sockfd);
		goto try_socket;
	}
	#endif
	//printf("bind secceed \n");
	
	struct timeval timeout={2,0};
	setsockopt(sockfd, SOL_SOCKET,SO_SNDTIMEO, &timeout, sizeof(timeout));

	int i;
	
	cmd[0] = 0xfe; //
	cmd[1] = 0x04; //flag of off CEC
	cmd[2] = num;//argc - 1; 
	
	for (i=0; i<num; i++)
	{
		cmd[i+3] = atoi(argv[i+1]);
		printf("cmd[%d] = %d\n", i+3, cmd[i+3]);
	}

	len = sendto(sockfd, &cmd, sizeof(cmd), \
			0, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if (len <= 0)
	{
		perror("sendto");
		printf("send len = %d \n", len);
		printf("server_broadcast sendto error \n");
	}
	else
	{
		printf("CEC OFF \n");
	}
	close(sockfd);
	return 0;
}




