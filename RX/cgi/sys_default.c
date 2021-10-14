#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define DEV_IO_NAME		"/dev/silan_testio"

//extern int reboot1(void);
typedef struct
{
	unsigned int addr;
	unsigned int data;
}SILANWORDR;

int main(void)
{
    FILE *config;
    unsigned char i;
   // system("rm /user/configs/config.conf");
    int ret;
    ret = unlink("/user/configs/config.conf");
    printf("content-Type:text/html\n\nsucceed");
    fflush(stdout);
    sync();
    
    usleep(10000);
    sleep(1);
    //reboot1();
    return 0;
   
}

