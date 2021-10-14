#include "linux_common.h"
#include <errno.h>
#include <netinet/in.h>

//about socket operation
/************************************************************************/
int CreateUDPSocket(void)
{
	int sockfd = socket(AF_INET,SOCK_DGRAM, 0); //UDP
    if (sockfd < 0)
    {
		printf("creat UDP socket failed \n");
		printf("errno is %d\n", errno);
		perror("socket");
		return -1;
	}
	return sockfd;
}

int CreateTCPSocket(void)
{
	int sockfd = socket(AF_INET,SOCK_STREAM, 0); //TCP
	if (sockfd < 0)
    {
		printf("create TCP socket failed\n");
		perror("socket");
		return -1;
	}
	return sockfd;
}

int SocketBindToAddr(int sockfd, const struct sockaddr *addr,
                socklen_t addrlen)
{
	int ret = -1;
	ret = bind(sockfd, addr, addrlen);
	if (ret)
	{
		printf("Socket bind to add Failed!\n");
        perror("bind");
		return -1;
	}
	return 0;
}

int ConnectToServer(int sockfd, const struct sockaddr *addr,
    socklen_t addrlen)
{
	int ret = -1;
	ret = connect(sockfd, addr, addrlen);
	if (ret)
	{
		printf("client socket connect to server Failed!\n");
        perror("connent");
		return -1;
	}
	return 0;
}

int SetReuseSocketAddr(const int sockfd)
{
	int reuse = 1;
	int ret = 0;
	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
	if(ret < 0)
	{
		printf("set socket address reuse failed\n");
		printf("errno is %d\n", errno);
		perror("setsockopt");
		return -1;
	}
	return 0;
}

int AddSocketToMulticast(const int sockfd, const char *multicast)
{
	int ret;

	//set multicast address 
	struct ip_mreq mreq;

	mreq.imr_multiaddr.s_addr = inet_addr(multicast);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	//add multicast group
	ret = setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
	if(ret < 0)
	{
		printf("set multicast error \n");
		printf("errno is %d \n", errno);
		perror("setsockopt");
		return -1;
	}
	return 0;
}

int AllowSocketSendBroadcast(const int sockfd)
{
	const int opt = 1;
	int ret = -1;
	ret = setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,(char*)&opt,sizeof(opt));
	if (ret < 0)
	{
		printf("Set socket as broadcast failed !!!\n");
		perror("setsockopt");
		return -1;
	}
	return 0;
}

int SendToDestAddr(int sockfd, const void *buf, size_t len, int flags,
    const struct sockaddr *dest_addr, socklen_t addrlen)
{
	int ret = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
	if(ret <= 0)
	{
		printf("Send data len <= 0 len= %d \n",len);
		printf("errno is %d\n",errno);
		perror("sendto");
		return -1;
	}
	return ret;
}

int RecvFromSrcAddr(int sockfd, void *buf, size_t len, int flags,
        struct sockaddr *src_addr, socklen_t *addrlen)
{
	int ret = -1;
	ret = recvfrom(sockfd, buf, len, 0, src_addr, addrlen);
	if (ret < 0)
	{
		printf("socker recv from UDP failed \n");
		perror("recvfrom");
		return -1;
	}
	return ret;
}

int SetRecvTimeOut(int sockfd, int sec, int usec)
{
	int ret = -1;
	struct timeval timeout = {sec,usec};
	ret = setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
	if (ret < 0)
	{
		printf("Set recv time out failed \n");
		perror("setsockopt");
		return -1;
	}
	return 0;
}

int SetSendTimeOut(int sockfd, int sec, int usec)
{
	int ret = -1;
	struct timeval timeout = {sec,usec};
	ret = setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
	if (ret < 0)
	{
		printf("Set send time out failed \n");
		perror("setsockopt");
		return -1;
	}
	return 0;
}

//about file operation
/*****************************************************************************/
int OpenFile(FILE* pFile,const char *pathname, const char *mode)
{
	pFile = fopen(pathname, mode);
	if (pFile == NULL)
	{
		printf("file %s open failed \n", pathname);
		printf("errno is %d\n", errno);
		perror("fopen");
		return -1;
	}
	return 0;
}

int WriteToFile(const void *ptr, size_t size, size_t nmemb,
                     FILE *stream)
{
	int ret = 0; 
	ret = fwrite(ptr, size, nmemb, stream);
	if (size <= 0)
	{
		printf("Write to file failed \n");
		printf("errno is %d\n", errno);
		perror("fwrite");
		return -1;
	}
	return ret;
}






