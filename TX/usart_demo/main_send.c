#include <stdio.h>
#include <stdlib.h>
#include "usart.h"

int main(int argc,char* argv[])
{
    int usart_id=0;
    char usart_dev[32]={0};
    int usart_fd=-1;
    char buf[]={"hello world!!!"};
    //int len=0;

    if(argc < 2)
    {
        printf("%s usart id 0/1/2\n",argv[0]);
        return -1;
    }
    usart_id=atoi(argv[1]);
    if(usart_id==0)
    {
        strcpy(usart_dev,UART_DEV_0);
    }
    else if(usart_id==1)
    {
        strcpy(usart_dev,UART_DEV_1);
    }
    else if(usart_id==2)
    {
        strcpy(usart_dev,UART_DEV_2);
    }
    else
    {
        return 0;
    }
    usart_fd=UART0_Open(usart_fd,usart_dev);
    if(usart_fd<0)
    {
        printf("open %s if failed!!!\n",usart_dev);
        return -1;
    }
    printf("open %s if ok!!!\n",usart_dev);
    if(UART0_Init(usart_fd, 115200,0,8,1,'N')== FALSE)
    {
        printf("init %s if failed!!!\n",usart_dev);
        return -1;
    }
    printf("init %s if ok!!!\n",usart_dev);
    while(1)
    {
        if(UART0_Send(usart_fd,buf,strlen(buf))==FALSE)
        {
            printf("usart send %s if failed!!!\n",usart_dev);
            return -1;
        }
        printf("send %s is ok\n",buf);
        sleep(2);
    }
    UART0_Close(usart_fd);
    return 0;
}