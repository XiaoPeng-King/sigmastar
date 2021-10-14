#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define REBOOT 	"/sbin/reboot"

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
    
    return 0;
}

