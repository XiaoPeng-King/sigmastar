
#ifndef _CUSEARLYINIT_PARA_H_
#define _CUSEARLYINIT_PARA_H_

enum {
    EARLYINIT_PARAM_TYPE_MASTER,
    EARLYINIT_PARAM_TYPE_CUST_ROSE
};

typedef struct
{
    unsigned short u16SnrEarlyFps;
    unsigned short u16SnrEarlyFlicker;
    unsigned long u32SnrEarlyShutter;
    unsigned long u32SnrEarlyGainX1024;
    unsigned long u32SnrEarlyDGain;
    unsigned long u32SnrEarlyShutterShort;
    unsigned long u32SnrEarlyGainX1024Short;
    unsigned long u32SnrEarlyDGainShort;
    unsigned short u16SnrEarlyAwbRGain;
    unsigned short u16SnrEarlyAwbGGain;
    unsigned short u16SnrEarlyAwbBGain;
} __attribute__ ((packed)) MasterEarlyInitParam_t;

typedef struct
{
    unsigned short u16SnrEarlyFps;
    unsigned short u16SnrEarlyFlicker;
    unsigned long u32SnrEarlyShutter;
    unsigned long u32SnrEarlyGainX1024;
    unsigned long u32SnrEarlyDGain;
    unsigned long u32DeBugLevel_AWB_PERIOD;
    unsigned long u32DeBugLevel_AeEarlyStageCnt;
    unsigned short u16SnrEarlyAwbRGain;
    unsigned short u16SnrEarlyAwbGGain;
    unsigned short u16SnrEarlyAwbBGain;
} __attribute__ ((packed)) CusEarlyInitParam_t;

typedef struct
{
    unsigned long u32Revision;
    unsigned long u32Size;

    union
    {
        MasterEarlyInitParam_t tMasSnrEarlyInitParam;

        // Add customize early-init parameter here, if any
        CusEarlyInitParam_t tCusSnrEarlyInitParam;

        unsigned char u8Data[64];
    } u;

} IspSensorEarlyInitParam_t;

#endif // _CUSEARLYINIT_PARA_H_
