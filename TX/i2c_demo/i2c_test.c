#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>

#define I2C_ADAPTER_STR_0     ("/dev/i2c-0")
#define I2C_ADAPTER_STR_1     ("/dev/i2c-1")
#define ES8156_CHIP_ADDR   (0x08)

static int s8I2cAdapterFd = -1;
static int bI2cInit = 0;

static const unsigned char u8Es8156InitSetting[][2] = {
    {0x02, 0x84},
    {0x00, 0x3c},
    {0x00, 0x1c},
    {0x02, 0x84},
    {0x0A, 0x01},
    {0x0B, 0x01},
    {0x0D ,0x14},
    {0x01, 0xC1},
    {0x18, 0x00},
    {0x08, 0x3F},
    {0x09, 0x02},
    {0x00, 0x01},
    {0x22, 0x00},
    {0x23, 0xCA},
    {0x24, 0x00},
    {0x25, 0x20},
    {0x14, 0xbf},//dac输出gain可以调低
    {0x11, 0x31},
};

static const unsigned char u8Es8156DeinitSetting[][2] = {
    {0x14, 0x00},
    {0x19, 0x02},
    {0x21, 0x1F},
    {0x22, 0x02},
    {0x25, 0x21},
    {0x25, 0x01},
    {0x25, 0x87},
    {0x18, 0x01},
    {0x09, 0x02},
    {0x09, 0x01},
    {0x08, 0x00},
};

int I2C_Init(unsigned int timeout, unsigned int retryTime)
{
    int s32Ret;
    if (0 == bI2cInit)
    {
        s8I2cAdapterFd = open((const char *)I2C_ADAPTER_STR_1, O_RDWR);
        if (-1 == s8I2cAdapterFd)
        {
            printf("Error: %s\n", strerror(errno));
            return -1;
        }

        s32Ret = ioctl(s8I2cAdapterFd, I2C_TIMEOUT, timeout);
        if (s32Ret < 0)
        {
            printf("Failed to set i2c time out param.\n");
            close(s8I2cAdapterFd);
            return -1;
        }

        s32Ret = ioctl(s8I2cAdapterFd, I2C_RETRIES, retryTime);
        if (s32Ret < 0)
        {
            printf("Failed to set i2c retry time param.\n");
            close(s8I2cAdapterFd);
            return -1;
        }

        bI2cInit = 1;
    }
    return 0;
}

int I2C_Deinit(void)
{
    if (1 == bI2cInit)
    {
        close(s8I2cAdapterFd);
        bI2cInit = 0;
    }
    return 0;
}

int I2C_GetFd(void)
{
    return s8I2cAdapterFd;
}


int I2C_WriteByte(unsigned char reg, unsigned char val, unsigned char i2cAddr)
{
    int s32Ret;
    unsigned char data[2];
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages;

    if (-1 == I2C_GetFd())
    {
        printf("ES7210 hasn't been call I2C_Init.\n");
        return -1;
    }

    memset((&packets), 0x0, sizeof(packets));
    memset((&messages), 0x0, sizeof(messages));

    // send one message
    packets.nmsgs = 1;
    packets.msgs = &messages;

    // fill message
    messages.addr = i2cAddr;   // codec reg addr
    messages.flags = 0;                 // read/write flag, 0--write, 1--read
    messages.len = 2;                   // size
    messages.buf = data;                // data addr

    // fill data
    data[0] = reg;
    data[1] = val;

    s32Ret = ioctl(I2C_GetFd(), I2C_RDWR, (unsigned long)&packets);
    if (s32Ret < 0)
    {
        printf("Failed to write byte to ES7210: %s.\n", strerror(errno));
        return -1;
    }

    return 0;
}

int I2C_ReadByte(unsigned char reg, unsigned char *val, unsigned char i2cAddr)
{
    int s32Ret;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];
    unsigned char tmpReg, tmpVal;

    if (-1 == I2C_GetFd())
    {
       printf("ES7210 hasn't been call I2C_Init.\n");
        return -1;
    }

    if (NULL == val)
    {
        printf("val param is NULL.\n");
        return -1;
    }

    tmpReg = reg;
    memset((&packets), 0x0, sizeof(packets));
    memset((&messages), 0x0, sizeof(messages));

    packets.nmsgs = 2;
    packets.msgs = messages;

    messages[0].addr = i2cAddr;
    messages[0].flags = 0;
    messages[0].len = 1;
    messages[0].buf = &tmpReg;

    tmpVal = 0;
    messages[1].addr = i2cAddr;
    messages[1].flags = 1;
    messages[1].len = 1;
    messages[1].buf = &tmpVal;

    s32Ret = ioctl(I2C_GetFd(), I2C_RDWR, (unsigned long)&packets);
    if (s32Ret < 0)
    {
        printf("Failed to read byte from ES7210: %s.\n", strerror(errno));
        return -1;
    }

    *val = tmpVal;
    return 0;
}


static int ES8156_WriteByte(unsigned char reg, unsigned char val)
{
    return I2C_WriteByte(reg, val, ES8156_CHIP_ADDR);
}

static int ES8156_ReadByte(unsigned char reg, unsigned char *val)
{
    return I2C_ReadByte(reg, val, ES8156_CHIP_ADDR);
}

static int ES8156_Init(void)
{
    int s32Ret;
    unsigned char val = 0;
    unsigned char reg = 0;
    unsigned int u32Index = 0;

    //I2C_Init(10, 5);
    s32Ret = ES8156_ReadByte(0xfe, &val);
    if (0 == s32Ret)
    {
        printf("i2c read chip addr 0x08 ID0 0xFE:%x.\n",val);
    }
    else
    {
        return s32Ret;
    }

    s32Ret = ES8156_ReadByte(0xfd, &val);
    if (0 == s32Ret)
    {
        printf("i2c read chip addr 0x08 ID1 0XFD:%x.\n", val);
    }
    else
    {
        return s32Ret;
    }

    for (u32Index = 0; u32Index < ((sizeof(u8Es8156InitSetting)) / (sizeof(u8Es8156InitSetting[0]))); u32Index++)
    {
        reg = u8Es8156InitSetting[u32Index][0];
        val = u8Es8156InitSetting[u32Index][1];

        s32Ret = ES8156_WriteByte(reg, val);
        if (0 != s32Ret)
        {
            return s32Ret;
        }
        printf("i2c write chip addr 0x08 ID %#X = %#x, is ok\n",reg,val);
    }
    printf("======================Init ES8156 success.========================\n");
    return 0;
}

static int ES8156_Deinit(void)
{
    int s32Ret;
    unsigned char val = 0;
    unsigned char reg = 0;
    unsigned int u32Index = 0;

    for (u32Index = 0; u32Index < ((sizeof(u8Es8156DeinitSetting)) / (sizeof(u8Es8156DeinitSetting[0]))); u32Index++)
    {
        reg = u8Es8156DeinitSetting[u32Index][0];
        val = u8Es8156DeinitSetting[u32Index][1];

        s32Ret = ES8156_WriteByte(reg, val);
        if (0 != s32Ret)
        {
            return s32Ret;
        }
    }
    //I2C_Deinit();
    printf("======================Deinit ES8156 success.========================\n");
    return 0;
}

int str_to_hex(const char* str,unsigned char* hex)
{
    int len=0;
    if(str==NULL)
    {
        return -1;
    }
    len=strlen(str);
    if(len==0)
    {
        return -1;
    }
    sscanf(str,"%hhX",hex);
    return 0;
}

int main(int argc ,char* argv[])
{   
    int s32Ret=0;
    unsigned char reg,chip_addr;
    unsigned char val=0;
    if(argc<2)
    {
        printf("%s [-s/-r/-w/-d] [chip addr] [reg] [val]\n",argv[0]);
        printf("%s -s \n",argv[0]);
        printf("%s -r [chip addr 0x08] [reg 0xfe]\n",argv[0]);
        printf("%s -w [chip addr] [reg] [val]\n",argv[0]);
        printf("%s -d \n",argv[0]);
        return -1;
    }
    I2C_Init(10, 5);
    if(strcmp("-s",argv[1])==0)
    {
        ES8156_Init();
    }
    else if(strcmp("-r",argv[1])==0)
    {
        str_to_hex(argv[2],&chip_addr);
        str_to_hex(argv[3],&reg);
        printf("chip_addr = %#X,reg = %#X\n",chip_addr,reg);
        s32Ret=I2C_ReadByte(reg, &val,chip_addr);
        if (0 != s32Ret)
        {
            printf("i2c read is failed!!!!\n");
            I2C_Deinit();
            return s32Ret;
        }
        printf("i2c read chip addr=%#X reg=%#X val=%#X\n",(unsigned char)chip_addr,(unsigned char)reg,val);
    }
    else if(strcmp("-w",argv[1])==0)
    {
        str_to_hex(argv[2],&chip_addr);
        str_to_hex(argv[3],&reg);
        str_to_hex(argv[4],&val);
        printf("chip_addr = %#X,reg = %#X,val=%#X\n",chip_addr,reg,val);

        s32Ret=I2C_WriteByte(reg,val,chip_addr);
        if (0 != s32Ret)
        {
            printf("i2c write is failed!!!!\n");
            I2C_Deinit();
            return s32Ret;
        }
        printf("i2c write chip addr=%#X reg=%#X val=%#X\n",(unsigned char)chip_addr,(unsigned char)reg,val);
    }
    else if(strcmp("-d",argv[1])==0)
    {
        ES8156_Deinit();
    }
    else
    {
        printf("%s [-s/-r/-w/-d] [chip addr] [reg] [val]\n",argv[0]);
        printf("%s -s \n",argv[0]);
        printf("%s -r [chip addr] [reg]\n",argv[0]);
        printf("%s -w [chip addr] [reg] [val]\n",argv[0]);
        printf("%s -d \n",argv[0]);
        return -1;
    }
    I2C_Deinit();
    return 0;
}
