#include <stdio.h>
#include <unistd.h>

#include "gpio.h"

#define TRUE 1
#define FALSE   0
#define TIME 5

#define I2C_SDA		6 //GPIO6
#define I2C_SCL		5 //GPIO5

#define INPUT 1
#define OUTPUT 0

#define HIGH 1
#define LOW 0

#if 0
#define SDA_INPUT	set_gpio_state(I2C_SDA, INPUT)
#define SDA_OUTPUT	set_gpio_state(I2C_SDA, OUTPUT)

#define SCL_INPUT	set_gpio_state(I2C_SCL, INPUT)
#define SCL_OUTPUT	set_gpio_state(I2C_SCL, OUTPUT)

#define SDA_HIGH	
#define SDA_LOW

#define SCL_HIGH
#define SCL_LOW
#endif



void create_gpio(unsigned char gpio_num)
{
    return;
}
void set_gpio_state(unsigned char gpio_num, unsigned char state)
{
    return;
}


//初始化用于模拟IIC的GPIO
void i2c_gpio_init(void)
{
	create_gpio(I2C_SDA);
	set_gpio_state(I2C_SDA, OUTPUT);
	set_gpio_value(I2C_SDA, HIGH);

	create_gpio(I2C_SCL);
	set_gpio_state(I2C_SCL, OUTPUT);
	set_gpio_value(I2C_SCL, HIGH);

    #if 0
	GPIO_openFd(I2C_SDA);
    GPIO_export(I2C_SDA);
    GPIO_setDir(I2C_SDA, GPIO_OUTPUT);
    GPIO_setValue(I2C_SDA, GPIO_HIG_STA);

    GPIO_openFd(I2C_SCL);
    GPIO_export(I2C_SCL);
    GPIO_setDir(I2C_SCL, GPIO_OUTPUT);
    GPIO_setValue(I2C_SCL, GPIO_HIG_STA);
	#endif
}

static void delay_us(unsigned int time)
{
   usleep(TIME);
}

//获得当前数据线的电平状态
static int get_SDA_level(void)
{
	int value = 0;

	set_gpio_state(I2C_SDA, INPUT);
    //GPIO_setDir(I2C_SDA, GPIO_INPUT);
    usleep(TIME);
	value = get_gpio_value(I2C_SDA);
	//GPIO_getValue(I2C_SDA, &value);
	usleep(TIME);
	
	return value;
}

//IIC 开始信号
void  gpio_i2c_start(void)
{
	set_gpio_state(I2C_SDA, OUTPUT);
	set_gpio_state(I2C_SCL, OUTPUT);
    //GPIO_setDir(I2C_SDA, GPIO_OUTPUT); //SDA output
    //GPIO_setDir(I2C_SCL, GPIO_OUTPUT); //SCL output
    
	set_gpio_value(I2C_SDA, HIGH);
	set_gpio_value(I2C_SCL, HIGH);
    //GPIO_setValue(I2C_SDA, GPIO_HIG_STA); //SDA = 1
    //GPIO_setValue(I2C_SCL, GPIO_HIG_STA); //SCL = 1 
    usleep(TIME);

	set_gpio_value(I2C_SDA, LOW);
    //GPIO_setValue(I2C_SDA, GPIO_LOW_STA); //SDA = 0
    usleep(TIME);
}

//IIC 结束信号
void  gpio_i2c_end(void)
{
	set_gpio_state(I2C_SDA, OUTPUT);
	set_gpio_state(I2C_SCL, OUTPUT);

    //GPIO_setDir(I2C_SDA, GPIO_OUTPUT);  //SDA output
    //GPIO_setDir(I2C_SCL, GPIO_OUTPUT);  //SCL output
    usleep(TIME);

	set_gpio_value(I2C_SDA, LOW);
	set_gpio_value(I2C_SCL, LOW);
    //GPIO_setValue(I2C_SDA, GPIO_LOW_STA); //SDA = 0
    //GPIO_setValue(I2C_SCL, GPIO_LOW_STA); //SCL = 0

    usleep(TIME);

	set_gpio_value(I2C_SCL, HIGH);
    //GPIO_setValue(I2C_SCL, GPIO_HIG_STA); //SCL = 1
    usleep(TIME);
	set_gpio_value(I2C_SDA, HIGH);
    //GPIO_setValue(I2C_SDA, GPIO_HIG_STA); //SDA = 1
    usleep(TIME);

	set_gpio_state(I2C_SDA, INPUT);
	set_gpio_state(I2C_SCL, INPUT);
    //GPIO_setDir(I2C_SDA, GPIO_INPUT);  //SDA input
    //GPIO_setDir(I2C_SCL, GPIO_INPUT);  //SCL input
    usleep(100);
}


/*
 * return : 
 *      1 : no ack
 *      0 : ack
*/
//读取IIC的相应信号
unsigned char gpio_i2c_read_ack(void) 
{
    unsigned char r;

	set_gpio_state(I2C_SDA, INPUT);
	set_gpio_value(I2C_SCL, HIGH);

    //GPIO_setDir(I2C_SDA, GPIO_INPUT); //SDA input
    //GPIO_setValue(I2C_SCL, GPIO_HIG_STA); //SCL = 1
    usleep(TIME);
    r = get_SDA_level(); //get SDA value
    usleep(TIME);

	set_gpio_value(I2C_SCL, LOW);
    //GPIO_setValue(I2C_SCL, GPIO_LOW_STA); //SCL = 0
    usleep(TIME);
    
    return r;
}

/* I2C发出ACK信号(读数据时使用) */  
void gpio_i2c_send_ack(void)
{
	set_gpio_state(I2C_SDA, OUTPUT);
    //GPIO_setDir(I2C_SDA, GPIO_OUTPUT);
    //GPIO_setDir(I2C_SCL, GPIO_OUTPUT);

	set_gpio_value(I2C_SCL, LOW);
	set_gpio_value(I2C_SDA, LOW);
    //GPIO_setValue(I2C_SCL, GPIO_LOW_STA); //SCL = 0
    //GPIO_setValue(I2C_SDA, GPIO_LOW_STA); //SDA = 0

    usleep(TIME);
	set_gpio_value(I2C_SCL, HIGH);
    //GPIO_setValue(I2C_SCL, GPIO_HIG_STA); //SCL = 1
    usleep(TIME);
	set_gpio_value(I2C_SCL, LOW);
    //GPIO_setValue(I2C_SCL, GPIO_LOW_STA); //SCL = 0
    usleep(TIME);
}

void gpio_i2c_send_nack(void)
{
	set_gpio_state(I2C_SDA, OUTPUT);
    //GPIO_setDir(I2C_SDA, GPIO_OUTPUT); //SDA output
    //GPIO_setDir(I2C_SCL, GPIO_OUTPUT);

	set_gpio_value(I2C_SCL, LOW);
	set_gpio_value(I2C_SDA, HIGH);
    //GPIO_setValue(I2C_SCL, GPIO_LOW_STA); //SCL = 0
    //GPIO_setValue(I2C_SDA, GPIO_HIG_STA); //SDA = 1

    usleep(TIME);
	set_gpio_value(I2C_SCL, HIGH);
    //GPIO_setValue(I2C_SCL, GPIO_HIG_STA); //SCL = 1
    usleep(TIME);
	set_gpio_value(I2C_SCL, LOW);
    //GPIO_setValue(I2C_SCL, GPIO_LOW_STA); //SCL = 0
    usleep(TIME);
}

/* I2C字节读 */  
unsigned char gpio_i2c_read_byte(void)  
{  
    int i;
    unsigned char r = 0x00;

    //GPIO_setDir(I2C_SDA, GPIO_INPUT); //SDA input

    usleep(TIME);
    for (i=7; i>=0; i--) 
    {
		set_gpio_value(I2C_SCL, HIGH);
        //GPIO_setValue(I2C_SCL, GPIO_HIG_STA); //SCL = 1
        usleep(TIME);
        r = ((r << 1) | get_SDA_level());      //从高位到低位依次准备数据进行读取  
        usleep(TIME);
        set_gpio_value(I2C_SCL, LOW);
		//GPIO_setValue(I2C_SCL, GPIO_LOW_STA); //SCL = 0
        usleep(TIME);
    }

    return r;  
}

/* I2C字节写 */
void gpio_i2c_write_byte(unsigned char byte)
{
    int i;

	set_gpio_state(I2C_SDA, OUTPUT);
	set_gpio_state(I2C_SCL, OUTPUT);
    //GPIO_setDir(I2C_SDA, GPIO_OUTPUT); //SDA output
    //GPIO_setDir(I2C_SCL, GPIO_OUTPUT); //SDA output
    usleep(TIME);
    for (i=7; i>=0; i--)
    {
		set_gpio_value(I2C_SCL, LOW);
        //GPIO_setValue(I2C_SCL, GPIO_LOW_STA); //SCL = 0
        usleep(TIME);
        if (byte & (1 << i)) //从高位到低位依次准备数据进行发送  
        {
			set_gpio_value(I2C_SDA, HIGH);
            //GPIO_setValue(I2C_SDA, GPIO_HIG_STA); //SDA = 1
        }
        else
        {
			set_gpio_value(I2C_SDA, LOW);
            //GPIO_setValue(I2C_SDA, GPIO_LOW_STA); //SDA = 0
        }
        usleep(TIME);
		set_gpio_value(I2C_SCL, HIGH);
        //GPIO_setValue(I2C_SCL, GPIO_HIG_STA); //SCL = 1
        usleep(TIME);
    }
	set_gpio_value(I2C_SCL, LOW);
    //GPIO_setValue(I2C_SCL, GPIO_LOW_STA); //SCL = 0
    
    usleep(TIME);
}

unsigned char i2c_ack_check(unsigned char ctrl_byte)
{
	gpio_i2c_start();
	gpio_i2c_write_byte(ctrl_byte);
	if(gpio_i2c_read_ack() == 0)
	{
		//sl_printf("i2c_read_ack() == 0.\n");
		// time delay here is not necessary, just to make waveforms more readable
		delay_us(30);
		//i2c_stop();
		//io_input(SDA);		// set SDA as input

		//io_input(SCL);		// set SCL as input
		return 0;
	}
	else
	{
		//sl_printf("i2c_read_ack() == 1.\n");
		// time delay here is to save computing resource
		delay_us(100);
		//io_input(SDA);		// set SDA as input
		//io_input(SCL);		// set SCL as input
		return 1;
	}
}





