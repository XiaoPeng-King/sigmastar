#ifndef __COMMON_H__
#define __COMMON_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

//socket 
int CreateUDPSocket(void);
int CreateTCPSocket(void);
int CreateIGMPSocket(void);

int SocketBindToAddr(int sockfd, const struct sockaddr *addr,
                socklen_t addrlen);
int ConnectToServer(int sockfd, const struct sockaddr *addr,
    socklen_t addrlen);

int SendToDestAddr(int sockfd, const void *buf, size_t len, int flags,
    const struct sockaddr *dest_addr, socklen_t addrlen);
int RecvFromSrcAddr(int sockfd, void *buf, size_t len, int flags,
        struct sockaddr *src_addr, socklen_t *addrlen);

int AcceptConnect(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int ListenSocket(int sockfd, int backlog);


int SetReuseSocketAddr(int sockfd);
int SetRecvTimeOut(int sockfd, int sec, int usec);
int SetSendTimeOut(int sockfd, int sec, int usec);
int AddSocketToMulticast(const int sockfd, const char *multicast);
int AllowSocketSendBroadcast(const int sockfd);

// file
FILE * OpenFile(const char *pathname, const char *mode);
size_t ReadFile(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t WriteToFile(const void *ptr, size_t size, size_t nmemb,
                     FILE *stream);

int GetShareMemoryID(key_t key, size_t size, int shmflg);
void *AttacheShareMemoryToProcess(int shmid, const void *shmaddr, int shmflg);


//system 
int CreateThread(pthread_t *thread, const pthread_attr_t *attr,
    void *(*start_routine) (void *), void *arg);

#endif
