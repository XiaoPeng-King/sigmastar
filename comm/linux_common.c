#include "linux_common.h"
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

//about socket operation
/************************************************************************/

/*
create a IPv4 socket :
example : 
	struct sockaddr_in  server_addr;
    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr(IP);
*/

int CreateUDPSocket(void)
{
	int sockfd = socket(AF_INET,SOCK_DGRAM, 0); //UDP
    if (sockfd < 0)
    {
		printf("creat UDP socket failed \n");
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

int CreateIGMPSocket(void)
{
	int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP); //IGMP
	if (sockfd < 0)
	{
		printf("create IGMP socket failed\n");
		perror("socket");
		return -1;
	}
	return sockfd;
}

//server need to bind an ip address for a socket
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

//client need to use this function to connect server
int ConnectToServer(int sockfd, const struct sockaddr *addr,
    socklen_t addrlen)
{
	int ret = -1;
	ret = connect(sockfd, (struct sockaddr *)addr, addrlen);
	if (ret < 0)
	{
		printf("client socket connect to server port Failed!\n");
        perror("connent");
		return -1;
	}
	return 0;
}

int ListenSocket(int sockfd, int backlog)
{
	int ret = -1;
	ret = listen(sockfd, backlog);
	if (ret < 0)
	{
		printf("Server Listen Failed!\n");
		perror("listen");
		return -1;
	}
	return 0;
}

int AcceptConnect(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret = -1;
	ret = accept(sockfd, addr, addrlen);
	if (ret < 0)
	{
		printf("Server accept failed");
		perror("accept");
		return -1;
	}
	return ret; //a new socket
}

int SetMulticastLoop(const int sockfd)
{
	int loop = 1; //1: on , 0: off
	int ret = -1;
	ret = setsockopt(sockfd,IPPROTO_IP,IP_MULTICAST_LOOP,&loop,sizeof(loop));
	if(ret < 0)
	{
		printf("set multicast loop failed !! \n");
		perror("setsockopt");
		return -1;
	}
	return ret;
}

int SetReuseSocketAddr(const int sockfd)
{
	int reuse = 1; //1 : on, 0 : off
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
	int ret = -1;
	ret = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
	if(ret <= 0)
	{
		printf("Sendto data len <= 0 len= %d \n",len);
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
	if (ret <= 0)
	{
		printf("socker recv from UDP failed \n");
		perror("recvfrom");
		return -1;
	}
	return ret;
}



//about file operation
/*****************************************************************************/
int IsExistent(const char *pathname)
{
	int ret = -1;
	ret = access(pathname, F_OK);
	if (ret < 0)
	{
		printf("file : %s doesn't exist \n", pathname);
		perror("access");
		return -1;
	}
	return 0;
}


FILE * OpenFile(const char *pathname, const char *mode)
{
	FILE *pFile;
	pFile = fopen(pathname, mode);
	if (pFile == NULL)
	{
		printf("file %s open failed \n", pathname);
		printf("errno is %d\n", errno);
		perror("fopen");
		return -1;
	}
	return pFile;
}

size_t ReadFile(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	int ret = -1;
	ret = fread(ptr, size, nmemb, stream);
	if (ret < 0)
	{
		printf("Read file failed \n");
		perror("fread");
		return -1;
	}
	return ret; //
}

size_t WriteToFile(const void *ptr, size_t size, size_t nmemb,
                     FILE *stream)
{
	int ret = 0; 
	ret = fwrite(ptr, size, nmemb, stream);
	if (size <= 0)
	{
		printf("Write to file failed \n");
		perror("fwrite");
		return -1;
	}
	return ret;
}

int Seek(int fd, off_t offset, int whence)
{
	int ret = -1;
	ret = lseek(fd, offset, whence);
	if (ret < 0)
	{
		printf("Seek failed \n");
		perror("lseek");
		return -1;
	}

	return ret;
}

//about process commnuication
//share memory
int GetShareMemoryID(key_t key, size_t size, int shmflg)
{
	int ret = -1;
	ret = shmget(key, size, shmflg);
	if (ret < 0)
	{
		printf("Get share memory ID failed \n");
		perror("shmget");
		return -1;
	}
	return ret;
}

void *AttacheShareMemoryToProcess(int shmid, const void *shmaddr, int shmflg)
{
	void *ret = NULL;
	ret = shmat(shmid, shmaddr, shmflg);
	if (ret < 0)
	{
		printf("Attache share memory to the process failed !!!\n");
		perror("shmat");
		return -1;
	}
	return ret;
}


//System function 
int CreateThread(pthread_t *thread, const pthread_attr_t *attr,
    void *(*start_routine) (void *), void *arg)
{
	int ret = -1;
	ret = pthread_create(thread, attr, start_routine, arg);
	if (ret != 0)
	{
		printf("create thread is failed \n");
		perror("pthread_create");
		exit(0);
	}
	return 0;
}

int Sleep()
{
	
}





