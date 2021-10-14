#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define DEV_IO_NAME		"/dev/silan_testio"
#define REBOOT 	"/sbin/reboot"

typedef struct
{
	unsigned int addr;
	unsigned int data;
}SILANWORDR;

int reboot1(void)
{
	int fd;
	int res = 0;
	SILANWORDR data;

	sleep(1);

	//system("cd /;umount /user");
	//system("cd /;umount /debug");

	fd = open(DEV_IO_NAME, O_RDWR);
	if(fd < 0){
		printf("Open the silan test io error\n");
	}

	data.addr = 3216674868UL; //0xbfba9034
	data.data = 4294967294UL; //fffffffe //reset
	res = ioctl(fd, 5, &data);
	if(res < 0){
		printf("### silan test io ioctl error!\n");
		return 1;
	}
	close(fd);

	return 0;
}

int main(void)
{
    FILE *config;
    unsigned char i;

    printf("content-Type:text/html\n\n");
    printf("succeed");
    
    fflush(stdout);
    usleep(10000);
    sleep(1);
    system(REBOOT);
    reboot1();
    
    return 0;
}

