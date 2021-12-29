#include <stdio.h>
#include <stdlib.h>
#include "usart.h"

int uart_main(void)
{
    printf("\n\n-----------------init uart watchdog-----------------\n\n");
    int ret = 0;
    int usart_id=1;
    char usart_dev[32]={0};
    int usart_fd=-1;
    char buf[]={"A"};
    //int len=0;
#if 0
    if(argc < 2)
    {
        printf("%s usart id 0/1/2\n",argv[0]);
        return -1;
    }
#endif
    //usart_id=atoi(argv[1]);
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
    usart_fd = UART0_Open(usart_dev);
    if(usart_fd < 0)
    {
        printf("open %s is failed!!!\n",usart_dev);
        return -1;
    }
    printf("usart fd = %d \n", usart_fd);
    printf("open %s is ok!!!\n",usart_dev);

    if(UART0_Init(usart_fd, 9600,0,8,1,'N')== FALSE)
    {
        printf("init %s is failed!!!\n",usart_dev);
        return -1;
    }
    printf("init %s is ok!!!\n",usart_dev);

    while(1)
    {
        if(UART0_Send(usart_fd,buf,strlen(buf))==FALSE)
        {
            printf("usart send %s is failed!!!\n",usart_dev);
            return -1;
        }
        //printf("\n\n-----------------uart watchdog send %s ok-----------------\n\n",buf);
        sleep(1);
    }
    UART0_Close(usart_fd);
    return 0;
}