#include "sharemem.h"
#include "gpio.h"

#define CEC_PORT	3

static char Hex1 = 0, Hex2 = 0;

char Enter_buff[] = "\n";

static unsigned char ASCII_To_Hex(unsigned char number)
{
	if (number >= '0' && number <= '9')
		return (number - 0x30);
 
	else if (number >= 'a' && number <= 'f')
		return ((number - 'a') + 10);
	
	else if (number >= 'A' && number <= 'F')
		return ((number - 'A') + 10);
 
	return (0);
}

void UART_CEC_TV_ON(void)
{
	cec_enable();
    printf("UART_CEC_TV_ON \n");
    char On_buff[] = {0xce,0x82,0x40,0x04};
	char rbuff[5] = {0};

    Hex1 = ASCII_To_Hex(share_mem->cec_control.cec_code_on[0]);
    Hex2 = ASCII_To_Hex(share_mem->cec_control.cec_code_on[1]);
    On_buff[2] = ((Hex1 << 4) | Hex2);

    Hex1 = ASCII_To_Hex(share_mem->cec_control.cec_code_on[3]);
    Hex2 = ASCII_To_Hex(share_mem->cec_control.cec_code_on[4]);
    On_buff[3] = ((Hex1 << 4) | Hex2);
    printf("On_buff: %x,%x,%x,%x\n",On_buff[0],On_buff[1],On_buff[2],On_buff[3]);
	
	int ret=-1;
    
	for(int i=0;i<3;i++)
    {
        sleep(1);

        ret = uart_write(CEC_PORT, 9600, On_buff, sizeof(On_buff));
        if(ret != 0)
        {
            printf(" UART CEC HEX write error ! \n");
        }

		ret = uart_write(CEC_PORT, 9600, Enter_buff, sizeof(On_buff));
        if(ret != 0)
        {
            printf(" UART CEC HEX write error ! \n");
        }
        
        ret = uart_read(CEC_PORT, 9600, rbuff,sizeof(rbuff));
        if(ret != 0)
        {
            printf(" UART CEC HEX Read error ! \n");
        }
        printf("rbuff: %x,%x,%x,%x\n",rbuff[0],rbuff[1],rbuff[2],rbuff[3]);

    }

    printf("rbuff: %d,%d,%d,%d\n",rbuff[0],rbuff[1],rbuff[2],rbuff[3]);

    if((rbuff[0]==130)&&(rbuff[1]==206))
    {
        printf("uart open tv command send success !\n");
        return 1;
    }
    else
    {
        return -1;
    }
}

void UART_CEC_TV_OFF(void)
{
	cec_enable();
    printf("UART_CEC_TV_OFF \n");

    char Off_buff[] = {0xce,0x36,0xff,0x36};
	char rbuff[5] = {0};

    Hex1 = ASCII_To_Hex(share_mem->cec_control.cec_code_off[0]);
    Hex2 = ASCII_To_Hex(share_mem->cec_control.cec_code_off[1]);
    Off_buff[2] = ((Hex1 << 4) | Hex2);

    Hex1 = ASCII_To_Hex(share_mem->cec_control.cec_code_off[3]);
    Hex2 = ASCII_To_Hex(share_mem->cec_control.cec_code_off[4]);
    Off_buff[3] = ((Hex1 << 4) | Hex2);

    printf("Off_buff: %x,%x,%x,%x\n",Off_buff[0],Off_buff[1],Off_buff[2],Off_buff[3]);

    memset(rbuff,0,sizeof(rbuff));
	
	int ret=-1;
	
	for(int i=0;i<3;i++)
    {
        sleep(1);

        ret = uart_write(CEC_PORT, 9600, Off_buff, sizeof(Off_buff));
        if(ret != 0)
        {
            printf(" UART CEC HEX write error ! \n");
        }

		ret = uart_write(CEC_PORT, 9600, Enter_buff, sizeof(Off_buff));
        if(ret != 0)
        {
            printf(" UART CEC HEX write error ! \n");
        }
        
        ret = uart_read(CEC_PORT, 9600, rbuff,sizeof(rbuff));
        if(ret != 0)
        {
            printf(" UART CEC HEX Read error ! \n");
        }
        printf("rbuff: %x,%x,%x,%x\n",rbuff[0],rbuff[1],rbuff[2],rbuff[3]);

    }

    printf("rbuff: %d,%d,%d,%d\n",rbuff[0],rbuff[1],rbuff[2],rbuff[3]);

    if((rbuff[0]==54)&&(rbuff[1]==206))
    {
        printf("uart off tv command send success !\n");
        return 1;
    }
    else
    {
        return -1;
    }

}




