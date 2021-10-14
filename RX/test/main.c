#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>


void main(void)
{
	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = inet_addr("239.255.2.2");       
	//mreq.imr_multiaddr.s_addr = inet_addr(multicast);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	printf("hello world \n");
}