#include <errno.h>
#include "uart.h"
#include <termios.h>

int uart_watchdog_main(void)
{
    int fd;
    int nread, nwrite, i;
    unsigned int ispeed,ospeed;
    char write_buff[8]="A";
	char read_buff[8]="";
    struct termios tmptio;
 
    if((fd=open_port(fd,2))<0)
    {
        perror("open_port error");
        return;
    }
    if((i=set_opt(fd,9600,8,'N',1))<0)
    {
        perror("set_opt error");
        return;
    }

	if( tcgetattr(fd,&tmptio)  !=  0) 
	{ 
		perror("SetupSerial 1");
		return -1;
	}
	ospeed = cfgetospeed(&tmptio);
	ispeed = cfgetispeed(&tmptio);

	printf("ospeed: %d,ispeed: %d\n",ospeed,ispeed);

	while (1)
	{
		nwrite=write(fd,write_buff,8);
        //printf("nwrite=%d,%s\n",nwrite,write_buff);

        nread=read(fd,read_buff,8);
        //printf("nread=%d,%s\n",nread,read_buff);
        sleep(1);
	}
    
    close(fd);

    return;
}