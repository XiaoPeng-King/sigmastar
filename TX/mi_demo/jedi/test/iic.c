#include <stdio.h>
#include <linux/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define FILE_NAME "/dev/i2c-1"


static int i2c_write(int fd,unsigned char slave_addr, unsigned char reg_addr, unsigned char value)
{
    unsigned char outbuf[2];
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];
   
    messages[0].addr = slave_addr;
    messages[0].flags = 0;
    messages[0].len = sizeof(outbuf);
    messages[0].buf = outbuf;
    /* The first byte indicates which register we‘ll write */
    outbuf[0] = reg_addr;
    /*
    * The second byte indicates the value to write. Note that for many
    * devices, we can write multiple, sequential registers at once by
    * simply making outbuf bigger.
    */
    outbuf[1] = value;
    /* Transfer the i2c packets to the kernel and verify it worked */
    packets.msgs = messages;
    packets.nmsgs = 1;
    if(ioctl(fd, I2C_RDWR, &packets) < 0)
    {
        perror("Unable to send data");
        return 1;
    }
    return 0;
}

int i2c_bytes_write(int fd,unsigned char slave_addr, unsigned char reg_addr, unsigned long int len, unsigned char *buf)
{
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];

    char *pbuf = malloc(sizeof(char)*(len+1));
    pbuf[0] = reg_addr;
    memcpy((pbuf+1), buf, len);

    messages[0].addr = slave_addr;
    messages[0].flags = 0;
    messages[0].len = len + 1;
    messages[0].buf = pbuf;

    /* Transfer the i2c packets to the kernel and verify it worked */
    packets.msgs = messages;
    packets.nmsgs = 1;

    if(ioctl(fd, I2C_RDWR, &packets) < 0)
    {
        perror("Unable to send data");
        return -1;
    }
    return 0;
}

static int i2c_read(int fd, unsigned char slave_addr, unsigned char reg_addr, unsigned char *value)
{
    unsigned char inbuf, outbuf;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];
    /*
    * In order to read a register, we first do a "dummy write" by writing
    * 0 bytes to the register we want to read from. This is similar to
    * the packet in set_i2c_register, except it‘s 1 byte rather than 2.
    */
    outbuf = reg_addr;
    messages[0].addr = slave_addr;
    messages[0].flags = 0;
    messages[0].len = sizeof(outbuf);
    messages[0].buf = &outbuf;

    /* The data will get returned in this structure */
    messages[1].addr = slave_addr;
    messages[1].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
    messages[1].len = sizeof(inbuf);
    messages[1].buf = &inbuf;
    
    /* Send the request to the kernel and get the result back */
    packets.msgs = messages;
    packets.nmsgs = 2;
    if(ioctl(fd, I2C_RDWR, &packets) < 0)
    {
        perror("Unable to send data");
        return 1;
    }
    *value = inbuf;
    return 0;
}

int i2c_bytes_read(int fd, unsigned char slave_addr, unsigned char reg_addr, unsigned long int len, unsigned char *pdata)
{
    unsigned char inbuf, outbuf;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    outbuf = reg_addr;

    messages[0].addr = slave_addr;
    messages[0].flags = 0;
    messages[0].len = sizeof(outbuf);
    messages[0].buf = &outbuf;

    /* The data will get returned in this structure */
    messages[1].addr = slave_addr;
    messages[1].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
    messages[1].len = len;
    messages[1].buf = pdata;

    /* Send the request to the kernel and get the result back */
    packets.msgs = messages;
    packets.nmsgs = 2;


    if(ioctl(fd, I2C_RDWR, &packets) < 0)
    {
        perror("Unable to send data");
        return -1;
    }

    return 0;
}

#if 0
int main(int argc, char **argv)
{
    int fd;

    unsigned int slave_addr=0, reg_addr=0, value = 0;
    if (argc < 4){
    printf("Usage:\n%s r[w] start_addr reg_addr [value]\n",argv[0]);
    return 0;
    }

    fd = open(FILE_NAME, O_RDWR);
    if (!fd)
    {
        printf("can not open file %s\n", FILE_NAME);
        return 0;
    }
    sscanf(argv[2], "%x", &slave_addr);
    sscanf(argv[3], "%x", &reg_addr);

    if(!strcmp(argv[1],"r"))
    {
        i2c_read(fd, slave_addr, reg_addr, (unsigned char*)&value);
    }
    else if(argc>4&&!strcmp(argv[1],"w"))
    {
        sscanf(argv[4], "%x", &value);
        i2c_write(fd, slave_addr, reg_addr, value);
    }

    close(fd);

    return 0;
}
#endif
#if 1
int main(int argc, char **argv)
{
    int fd;

    unsigned int slave_addr=0x54, reg_addr=0x00;
    unsigned char write_buff[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00};
    unsigned char read_buff[10] = {0};

    fd = open(FILE_NAME, O_RDWR);
    if (!fd)
    {
        printf("can not open file %s\n", FILE_NAME);
        return 0;
    }

    while (1)
    {
        //i2c_write(fd, slave_addr, reg_addr, write_buff);
        i2c_bytes_write(fd, slave_addr, reg_addr, 10, (unsigned char*) write_buff);
        //i2c_read(fd, slave_addr, reg_addr, (unsigned char*)&read_buff);
        //sleep(1);
        i2c_bytes_read(fd, slave_addr, reg_addr, 10, (unsigned char*) &read_buff);
        printf("read buffer = %s \n", read_buff);
        //sleep(1);
        //read_buff = 0;
    }
    

    close(fd);

    return 0;
}
#endif