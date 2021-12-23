#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define SW_IP_READ 0x100001
#define SW_IP_WRITE 0x100002
#define SW_IP_SET_IN 0x100003
#define SW_IP_SET_OUT 0x100004

#define SW_1    4 //GPIO_4
#define SW_2    3 //GPIO_3
#define SW_3    6 //GPIO_6
#define SW_4    5 //GPIO_5
#define SW_5    9 //GPIO_9


typedef enum
{
	SW_GPIO_2 = 2,
	SW_GPIO_3,
	SW_GPIO_4,
    SW_GPIO_5,
    SW_GPIO_6,
    SW_GPIO_7,
    SW_GPIO_9 = 9,
    SW_PM_GPIO_0=62,
    SW_PM_GPIO_1=63,
    SW_PM_GPIO_3=65,
    SW_PM_GPIO_9=69
} SW_GPIO_E;

typedef struct _reg_cmd_t {
    SW_GPIO_E gpio;
    int data;
}reg_cmd_t;
extern unsigned char g_Exit;

static char get_gpio_value(const unsigned char gpio)
{
    char value = -1;
    int fd_gpio = -1;
    reg_cmd_t gpio_info;
#if 1
    if((gpio > 9 && gpio < 62) || gpio > 69 || gpio < 2 || gpio == 64 || gpio == 66 || gpio == 67|| gpio == 68)
    {
        printf("get gpio error ,gpio is <2-7/62/63/65/69> \n");
        return -1;
    }
#endif
    fd_gpio = open("/dev/SW_IP", O_RDWR);
    if (fd_gpio < 0)
    {
        printf("open /dev/SW_IP is failed!!\n");
        return -1;
    }
    memset(&gpio_info,0,sizeof(reg_cmd_t));
    gpio_info.gpio=gpio;
    gpio_info.data=0;

    ioctl(fd_gpio,SW_IP_SET_IN,&gpio_info);
    printf("set gpio in, gpio[%d] = %d\n\n",gpio_info.gpio,gpio_info.data);

    ioctl(fd_gpio,SW_IP_READ,&gpio_info);
    printf("read gpio %d = %d\n\n",gpio_info.gpio,gpio_info.data);

    value = gpio_info.data;

    close(fd_gpio);

    return value;
}

static int set_gpio_value(const unsigned char gpio, unsigned char value)
{
    int fd_gpio = -1;
    reg_cmd_t gpio_info;

     if((gpio > 9 && gpio < 62) || gpio > 69 || gpio < 2 || gpio == 64 || gpio == 66 || gpio == 67|| gpio == 68)
    {
        printf("get gpio error ,gpio is <2-7/62/63/65/69> \n");
        return -1;
    }

    fd_gpio = open("/dev/SW_IP", O_RDWR);
    if (fd_gpio < 0)
    {
        printf("open /dev/SW_IP is failed!!\n");
        return -1;
    }

    memset(&gpio_info,0,sizeof(reg_cmd_t));
    gpio_info.gpio=gpio;
    gpio_info.data=0;

    if(value < 0 || value > 1)
    {
        printf("Usage: %d -w <2-7,62/63/65/69> <in/out> <write value 0|1>\n", value);
        close(fd_gpio);
        return -1;
    }

    ioctl(fd_gpio,SW_IP_WRITE,&gpio_info);
    printf("write gpio %d = %d\n\n",gpio_info.gpio,gpio_info.data);
    
    gpio_info.data = value;
    ioctl(fd_gpio,SW_IP_SET_OUT,&gpio_info);
    printf("set gpio out, gpio[%d] = %d\n\n",gpio_info.gpio,gpio_info.data);
    
    close(fd_gpio);
    return 0;
}

#if 0
int main(int argc,char* argv[])
{
    int sw_ip_fd=-1;
    int cmd_type=0;
    int gpio=0;
    int value=0;
    reg_cmd_t gpio_info;

    if(argc < 3)
    {
        printf("Usage: %s <-r/-w/-s> <2-7,62/63/65/69> <in/out> <value 0|1> \n", argv[0]);
        return -1;
    }
    sw_ip_fd=open("/dev/SW_IP",O_RDWR);
    if(sw_ip_fd<0)
    {
        printf("open /dev/SW_IP is failed!!\n");
        return -1;
    }

    if(strcmp("-r",argv[1])==0)
    {
        if(argc < 3)
        {
            printf("Usage: %s <-r/-w/-s> <2-7,62/63/65/69> <in/out> <write value 0|1>\n", argv[0]);
            return -1;
        }
        cmd_type=0;
    }
    else if(strcmp("-w",argv[1])==0)
    {
        if(argc < 4)
        {
            printf("Usage: %s <-r/-w/-s> <2-7,62/63/65/69> <in/out> <write value 0|1>\n", argv[0]);
            return -1;
        }
        cmd_type=1;
    }
    else if(strcmp("-s",argv[1])==0)
    {
        if(argc < 4)
        {
            printf("Usage: %s <-r/-w/-s> <2-7,62/63/65/69> <in/out> <write value 0|1>\n", argv[0]);
            return -1;
        }
        cmd_type=2;
    }
    else 
    {
        printf("cmd is failed!!,cmd is -r/-w/-s\n");
        return -1;
    }

    gpio=atoi(argv[2]);
    if((gpio > 7 && gpio < 62) || gpio > 69 || gpio < 2 || gpio == 64 || gpio == 66 || gpio == 67|| gpio == 68)
    {
        printf("cmd is failed!!,gpio is <2-7/62/63/65/69> \n");
        close(sw_ip_fd);
        return -1;
    }

    memset(&gpio_info,0,sizeof(reg_cmd_t));
    gpio_info.gpio=gpio;
    gpio_info.data=0;
    if(cmd_type==0)
    {
        gpio_info.data=0;
        ioctl(sw_ip_fd,SW_IP_READ,&gpio_info);
        printf("read gpio %d = %d\n\n",gpio_info.gpio,gpio_info.data);
    }
    else if(cmd_type==1)
    {
        value=atoi(argv[3]);
        if(value<0 || value >1)
        {
            printf("Usage: %s -w <2-7,62/63/65/69> <in/out> <write value 0|1>\n", argv[0]);
            close(sw_ip_fd);
            return -1;
        }
        gpio_info.data=value;
        ioctl(sw_ip_fd,SW_IP_WRITE,&gpio_info);
        printf("write gpio %d = %d\n\n",gpio_info.gpio,gpio_info.data);
    }
    else if(cmd_type==2)
    {
        if(strcmp("in",argv[3])==0)
        {
            ioctl(sw_ip_fd,SW_IP_SET_IN,&gpio_info);
            printf("set gpio in, gpio[%d] = %d\n\n",gpio_info.gpio,gpio_info.data);
        }
        else if(strcmp("out",argv[3])==0)
        {
            value=atoi(argv[4]);
            if(value<0 || value >1)
            {
                printf("Usage: %s -w <2-7,62/63/65/69> <in/out> <write value 0|1>\n", argv[0]);
                close(sw_ip_fd);
                return -1;
            }
            gpio_info.data=value;
            ioctl(sw_ip_fd,SW_IP_SET_OUT,&gpio_info);
            printf("set gpio out, gpio[%d] = %d\n\n",gpio_info.gpio,gpio_info.data);
        }
        else
        {
            printf("Usage: %s -w <2-7,62/63/65/69> <in/out> <write value 0|1>\n", argv[0]);
            close(sw_ip_fd);
            return -1;
        }
    }

    close(sw_ip_fd);
    return 0;
}
#endif


static char detect_sw_id(void)
{
	char id = 0;
	char flag = 0;

    flag = get_gpio_value(SW_5);
	id = (id | (flag << 0));

	flag = get_gpio_value(SW_4);
	id = (id | (flag << 1));

	flag = get_gpio_value(SW_3);
	id = (id | (flag << 2));

	flag = get_gpio_value(SW_2);
	id = (id | (flag << 3));

	flag = get_gpio_value(SW_1);
	id = (id | (flag << 4));

	//printf("detect sw id : 0x%x \n", id);

	return id;
}

unsigned char IP_check(void)
{
    int ip = 0, temp = -1;

    while (1)
    {
        ip = detect_sw_id();
        if (ip != temp)
        {
            temp = ip;
        }
        else
        {
            sleep(1);
            printf("ip : %d \n", ip);
            break;
        }
    }
    return ip+200;
}



