#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "sharemem.h"
#include "version.h"
#include "gpio_iic.h"
#include "gpio.h"

//unsigned char numbers[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e};  //0~f duanxuanxinhao
unsigned char numbers[]={0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10,0x08,0x03,0x46,0x21,0x06,0x0e};  //0~f duanxuanxinhao

unsigned int flag1,flag2;

unsigned int ip = 0;

void digit_led_i2c_write(unsigned char DeviceAddr, unsigned char pBuffer)
{
	gpio_i2c_start();

	gpio_i2c_write_byte(DeviceAddr);    // 1.发送数据命令(01001000)
    if (1 == gpio_i2c_read_ack())
	{
		printf("read ack error \n");
	}

	gpio_i2c_write_byte(pBuffer);   // 1.发送数据命令(01001000)
	if (1 == gpio_i2c_read_ack())
	{
		printf("read ack error \n");
	}
    usleep(10000);
    
	gpio_i2c_end();
}

//十位
static int H_anjian(void)
{
#if 1
    int tmp;
    
    flag1++;
    printf(" flag1++ : %d\n",flag1);
    if ((flag2 >= 4) && (flag1 > 2)) //24
    {
        flag1 = 0;
    }
    if ((flag2 == 0) && (flag1 > 2)) //00
    {
        flag1 = 1;
    }

    digit_led_i2c_write(0x68,numbers[flag1]);
    tmp=(flag1*10)+flag2;
                
    return tmp;
#endif
}

//个位
static int U_anjian(void)
{
    #if 1
    int tmp;
    
    //strcpy(rx_ip,share_mem->sm_eth_setting.strEthIp);
    //sscanf(rx_ip,"192.168.1.%d",&n);
    //printf("ip is : %d\n",n);
    //flag1 = n/10;
    //flag2 = n%10;

    flag2++;
    printf(" flag2++ : %d\n",flag2);
    if ((flag1==0)&&(flag2>9)) //
    {
        flag2=1;
    }
    if ((flag1==2)&&(flag2 > 4))
    {
        flag2 = 0;
    }
    if ((flag1!=0)&&(flag1!=2))
    {
        if(flag2>9)
            flag2=0;
    }
    digit_led_i2c_write(0x6A,numbers[flag2]);
    tmp=(flag1*10)+flag2;
   
    return tmp;
    #endif
}

void *digit_led_main(void)
{
    char tx_ip[20];
    char str[20];
    char n;
    char tmp, count;

#if 1
    printf("share_mem->sm_eth_setting.strEthIp : %s \n", share_mem->sm_eth_setting.strEthIp);
    strcpy(tx_ip,share_mem->sm_eth_setting.strEthIp);
    printf("%s\n", tx_ip);
    sscanf(tx_ip,"192.168.36.%d\n",&n);
    printf("ip is : %d\n",n);
    n-=200;
#endif

#if 0
    inet_pton(AF_INET, share_mem->sm_eth_setting.strEthIp, &rx_ip);
	rx_ip = ntohl(rx_ip); //host
    printf("%d \n", rx_ip);
	rx_ip &= 0xFF;
    printf("%d \n", rx_ip);
#endif
    flag1 = n/10;
    flag2 = n%10;

    i2c_gpio_init();
    //key_init();
    digit_led_i2c_write(0x48,0x01);
    digit_led_i2c_write(0x68,numbers[flag1]);
    digit_led_i2c_write(0x6A,numbers[flag2]);
    ip = n;
    tmp = ip;
    while (1)
    {
        usleep(100000);
        if (!get_up1_value())
        {
            ip = U_anjian();
        }
        if (!get_up2_value())
        {
            ip = H_anjian();
        }
        if (ip != tmp)
        {
            sprintf(str, "192.168.36.%d", ip+200);
            printf(str);
            #if 1
            if (strcmp(str, share_mem->sm_eth_setting.strEthIp))
            {
                strcpy(share_mem->sm_eth_setting.strEthIp, str);
                printf(share_mem->sm_eth_setting.strEthIp);
                AppWriteCfgInfotoFile();
                init_eth(); //ip configer
            }
            #endif
            tmp = ip;
        }
    }

    return 0;
}

