#ifndef __COMMON_H__
#define __COMMON_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>

int SendToDestAddr(int sockfd, const void *buf, size_t len, int flags,
    const struct sockaddr *dest_addr, socklen_t addrlen);
int CreateUDPSocket(void);
int SetReuseSocketAddr(int sockfd);
int SetRecvTimeOut(int sockfd, int sec, int usec);
int SetSendTimeOut(int sockfd, int sec, int usec);


int OpenFile(FILE* pFile,const char *pathname, const char *mode);
int WriteToFile(const void *ptr, size_t size, size_t nmemb,
                     FILE *stream);





#endif
