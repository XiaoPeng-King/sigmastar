/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

  Unless otherwise stipulated in writing, any and all information contained
 herein regardless in any format shall remain the sole proprietary of
 Sigmastar Technology Corp. and be kept in strict confidence
 (��Sigmastar Confidential Information��) by the recipient.
 Any unauthorized act including without limitation unauthorized disclosure,
 copying, use, reproduction, sale, distribution, modification, disassembling,
 reverse engineering and compiling of the contents of Sigmastar Confidential
 Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
 rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>

#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_ai.h"
#include "mi_ao.h"
#include "mi_sensor.h"

#include <sys/ioctl.h>
#include <errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "audio_ringbuffer.h"

#define WAV_G711A 	(0x06)
#define WAV_G711U 	(0x07)
#define WAV_G726 	(0x45)
#define WAV_PCM  	(0x1)

#define G726_16 	(2)
#define G726_24 	(3)
#define G726_32 	(4)
#define G726_40 	(5)

#define AI_DMIC_CHN_MAX		(2)


#define AI_AMIC_CHN_MAX 	(2)
#define AI_I2S_NOR_CHN 		(2)

#define AI_DEV_ID_MAX   (3)
#define AO_DEV_ID_MAX   (2)



#define AI_DEV_AMIC     (0)
#define AI_DEV_DMIC     (1)
#define AI_DEV_I2S_RX   (2)
#define AI_DEV_LineIn   (3)

#define AO_DEV_LineOut  (0)
#define AO_DEV_I2S_TX   (1)


#define AI_VOLUME_AMIC_MIN      (0)
#define AI_VOLUME_AMIC_MAX      (21)
#define AI_VOLUME_LINEIN_MAX    (7)


#define AI_VOLUME_DMIC_MIN      (-60)
#define AI_VOLUME_DMIC_MAX      (30)

#define AO_VOLUME_MIN           (-60)
#define AO_VOLUME_MAX           (30)

#define MI_AUDIO_SAMPLE_PER_FRAME	(1024)

#define DMA_BUF_SIZE_8K     (8000)
#define DMA_BUF_SIZE_16K    (16000)
#define DMA_BUF_SIZE_32K    (32000)
#define DMA_BUF_SIZE_48K    (48000)

#define AI_DMA_BUFFER_MAX_SIZE	(256 * 1024)
#define AI_DMA_BUFFER_MID_SIZE	(128 * 1024)
#define AI_DMA_BUFFER_MIN_SIZE	(64 * 1024)

#define AO_DMA_BUFFER_MAX_SIZE	(256 * 1024)
#define AO_DMA_BUFFER_MID_SIZE	(128 * 1024)
#define AO_DMA_BUFFER_MIN_SIZE	(64 * 1024)

#define MIU_WORD_BYTE_SIZE	(8)
#define TOTAL_BUF_DEPTH		(8)

extern unsigned char g_Exit;

typedef enum
{
	E_AI_SOUND_MODE_MONO = 0,
	E_AI_SOUND_MODE_STEREO,
	E_AI_SOUND_MODE_QUEUE,
} AiSoundMode_e;

#define ExecFunc(func, _ret_) \
do{	\
	MI_S32 s32TmpRet;	\
    printf("%d Start test: %s\n", __LINE__, #func);\
    s32TmpRet = func;	\
    if (s32TmpRet != _ret_)\
    {\
        printf("AUDIO_TEST [%d] %s exec function failed, result:0x%x\n",__LINE__, #func, s32TmpRet);\
        return 1;\
    }\
    else\
    {\
        printf("AUDIO_TEST [%d] %s  exec function pass\n", __LINE__, #func);\
    }\
    printf("%d End test: %s\n", __LINE__, #func);	\
}while(0);

#define ExecFuncNoExit(func, _ret_, __ret) \
do{	\
	MI_S32 s32TmpRet;	\
    printf("%d Start test: %s\n", __LINE__, #func);\
    s32TmpRet = func;	\
    __ret = s32TmpRet;	\
    if (s32TmpRet != _ret_)\
    {\
        printf("AUDIO_TEST [%d] %s exec function failed, result:0x%x\n",__LINE__, #func, s32TmpRet);\
    }\
    else\
    {\
        printf("AUDIO_TEST [%d] %s  exec function pass\n", __LINE__, #func);\
        printf("%d End test: %s\n", __LINE__, #func);	\
    }\
}while(0);

/*=============================================================*/
// Global Variable definition
/*=============================================================*/
typedef enum
{
    E_SOUND_MODE_MONO = 0, /* mono */
    E_SOUND_MODE_STEREO = 1, /* stereo */
} SoundMode_e;

typedef enum
{
    E_SAMPLE_RATE_8000 = 8000, /* 8kHz sampling rate */
    E_SAMPLE_RATE_16000 = 16000, /* 16kHz sampling rate */
    E_SAMPLE_RATE_32000 = 32000, /* 32kHz sampling rate */
    E_SAMPLE_RATE_48000 = 48000, /* 48kHz sampling rate */
} SampleRate_e;

typedef enum
{
    E_AENC_TYPE_G711A = 0,
    E_AENC_TYPE_G711U,
    E_AENC_TYPE_G726_16,
    E_AENC_TYPE_G726_24,
    E_AENC_TYPE_G726_32,
    E_AENC_TYPE_G726_40,
    PCM,
} AencType_e;

typedef struct WAVE_FORMAT
{
    signed short wFormatTag;
    signed short wChannels;
    unsigned int dwSamplesPerSec;
    unsigned int dwAvgBytesPerSec;
    signed short wBlockAlign;
    signed short wBitsPerSample;
} WaveFormat_t;

typedef struct WAVEFILEHEADER
{
    char chRIFF[4];
    unsigned int  dwRIFFLen;
    char chWAVE[4];
    char chFMT[4];
    unsigned int  dwFMTLen;
    WaveFormat_t wave;
    char chDATA[4];
    unsigned int  dwDATALen;
} WaveFileHeader_t;

typedef struct AiOutFilenName_s
{
    MI_S8 *ps8OutputPath;
    MI_S8 *ps8OutputFile;
    MI_S8 *ps8SrcPcmOutputFile;
    MI_AUDIO_DEV AiDevId;
    MI_AUDIO_Attr_t stAiAttr;
    MI_BOOL bSetVqeWorkingSampleRate;
    MI_AUDIO_SampleRate_e eVqeWorkingSampleRate;
    MI_BOOL bEnableRes;
    MI_AUDIO_SampleRate_e eOutSampleRate;
    MI_BOOL bEnableAenc;
    MI_AI_AencConfig_t stSetAencConfig;
    MI_AI_VqeConfig_t stSetVqeConfig;
    MI_BOOL bEnableBf;
} AiOutFileName_t;

typedef struct AiChnPriv_s
{
    MI_AUDIO_DEV AiDevId;
    MI_AI_CHN AiChn;
    MI_S32 s32Fd;
    MI_U32 u32TotalSize;
    MI_U32 u32SrcPcmTotalSize;
    MI_U32 u32ChnCnt;
    MI_BOOL bEnableAed;
    pthread_t tid;
    MI_S32 s32SrcPcmFd;
} AiChnPriv_t;

static MI_BOOL  bEnableAI = FALSE;
static MI_U8*   pu8AiOutputPath = NULL;
static MI_BOOL  bAiEnableVqe = FALSE;
static MI_BOOL  bAiEnableHpf = FALSE;
static MI_BOOL  bAiEnableAgc = FALSE;
static MI_BOOL  bAiEnableNr = FALSE;
static MI_BOOL  bAiEnableEq = FALSE;
static MI_BOOL  bAiEnableAec = FALSE;
static MI_BOOL  bAiEnableResample = FALSE;
static MI_BOOL	bAiEnableSsl = FALSE;
static MI_BOOL	bAiEnableBf = FALSE;
static MI_AUDIO_SampleRate_e eAiOutputResampleRate = E_MI_AUDIO_SAMPLE_RATE_INVALID;
static MI_BOOL  bAiEnableAenc = FALSE;
static MI_AUDIO_AencType_e eAiAencType = E_MI_AUDIO_AENC_TYPE_INVALID;
static MI_AUDIO_G726Mode_e eAiAencG726Mode = E_MI_AUDIO_G726_MODE_INVALID;
static SoundMode_e eAiWavSoundMode = E_SOUND_MODE_MONO;
static AencType_e eAiWavAencType = PCM;
static MI_BOOL  bAiEnableAed = FALSE;
static MI_U32   u32AiChnCnt = 0;
static MI_BOOL  bAiEnableHwAec = FALSE;
static MI_S32   s32AiVolume = 0;
static MI_BOOL  bAiSetVolume = FALSE;
static MI_AUDIO_DEV AiDevId = -1;
static MI_AUDIO_SampleRate_e eAiWavSampleRate = E_MI_AUDIO_SAMPLE_RATE_INVALID;
static MI_AUDIO_SampleRate_e eAiSampleRate = E_MI_AUDIO_SAMPLE_RATE_INVALID;
static AiSoundMode_e eAiSoundMode = E_AI_SOUND_MODE_MONO;
static MI_BOOL 	bAiI2sNormalMode = TRUE;
static MI_U32	u32MicDistance = 0;
static MI_U32 	u32VqeWorkingSampleRate = E_MI_AUDIO_SAMPLE_RATE_INVALID;
static MI_BOOL  bAiDumpPcmData = FALSE;
static MI_BOOL  bAiSetBfDoa = FALSE;
static MI_S32   s32AiBfDoa = 0;

static MI_BOOL  bEnableAO = FALSE;
static MI_U8*   pu8AoInputPath = NULL;
static MI_BOOL  bAoEnableVqe = FALSE;
static MI_BOOL  bAoEnableHpf = FALSE;
static MI_BOOL  bAoEnableAgc = FALSE;
static MI_BOOL  bAoEnableNr = FALSE;
static MI_BOOL  bAoEnableEq = FALSE;
static MI_BOOL  bAoEnableResample = FALSE;
static MI_BOOL  bAoEnableAdec = FALSE;
static MI_AUDIO_SampleRate_e eAoOutSampleRate = E_MI_AUDIO_SAMPLE_RATE_INVALID;
static MI_AUDIO_SampleRate_e eAoInSampleRate = E_MI_AUDIO_SAMPLE_RATE_INVALID;
static MI_S32   s32AoVolume = 0;
static MI_BOOL  bAoSetVolume = FALSE;
static MI_AUDIO_DEV AoDevId = -1;

static MI_U32   u32RunTime = 0;
static MI_BOOL  bAiExit = FALSE;
static MI_BOOL  bAoExit = FALSE;

static MI_S32   AiChnFd[MI_AUDIO_MAX_CHN_NUM] = {[0 ... MI_AUDIO_MAX_CHN_NUM-1] = -1};
static MI_S32   AiChnSrcPcmFd[MI_AUDIO_MAX_CHN_NUM] = {[0 ... MI_AUDIO_MAX_CHN_NUM-1] = -1};

static AiChnPriv_t stAiChnPriv[MI_AUDIO_MAX_CHN_NUM];

static MI_S32   AoReadFd = -1;
static WaveFileHeader_t stWavHeaderInput;
static MI_AO_CHN AoChn = 0;
static MI_S32 s32NeedSize = 0;
static pthread_t Aotid;

MI_U8 u8TempBuf[MI_AUDIO_SAMPLE_PER_FRAME * 4];
static MI_U32   res[100];

MI_AUDIO_HpfConfig_t stHpfCfg = {
    .eMode = E_MI_AUDIO_ALGORITHM_MODE_USER,
    .eHpfFreq = E_MI_AUDIO_HPF_FREQ_150,
};

MI_AI_AecConfig_t stAecCfg = {
    .bComfortNoiseEnable = FALSE,
    .s16DelaySample = 0,
    .u32AecSupfreq = {4, 6, 36, 49, 50, 51},
    .u32AecSupIntensity = {5, 4, 4, 5, 10, 10, 10},
};

MI_AUDIO_AnrConfig_t stAnrCfg = {
    .eMode = E_MI_AUDIO_ALGORITHM_MODE_MUSIC,
    .u32NrIntensity = 15,
    .u32NrSmoothLevel = 10,
    .eNrSpeed = E_MI_AUDIO_NR_SPEED_MID,
};

MI_AUDIO_AgcConfig_t stAgcCfg = {
    .eMode = E_MI_AUDIO_ALGORITHM_MODE_USER,
    .s32NoiseGateDb = -60,
    .s32TargetLevelDb = -3,
    .stAgcGainInfo = {
        .s32GainInit = 0,
        .s32GainMax = 20,
        .s32GainMin = 0,
    },
    .u32AttackTime = 1,
    .s16Compression_ratio_input = {-80, -60, -40, -25, 0},
    .s16Compression_ratio_output = {-80, -30, -15, -10, -3},
    .u32DropGainMax = 12,
    .u32NoiseGateAttenuationDb = 0,
    .u32ReleaseTime = 3,
};

MI_AUDIO_EqConfig_t stEqCfg = {
    .eMode = E_MI_AUDIO_ALGORITHM_MODE_USER,
    .s16EqGainDb = {[0 ... 128] = 3},
};

MI_AI_AedConfig_t stAedCfg = {
    .bEnableNr = TRUE,
    .eSensitivity = E_MI_AUDIO_AED_SEN_HIGH,
    .s32OperatingPoint = -5,
    .s32VadThresholdDb = -40,
    .s32LsdThresholdDb = -15,
};

MI_AI_SslInitAttr_t stSslInit = {
	.bBfMode = FALSE,
};

MI_AI_SslConfigAttr_t stSslConfig = {
	.s32Temperature = 25,
	.s32NoiseGateDbfs = -40,
	.s32DirectionFrameNum = 300,
};

MI_AI_BfInitAttr_t stBfInit = {
	.u32ChanCnt = 2,
};

MI_AI_BfConfigAttr_t stBfConfig = {
	.s32Temperature = 25,
	.s32NoiseGateDbfs = -40,
	.s32NoiseSupressionMode = 8,
	.s32NoiseEstimation = 1,
	.outputGain = 0.7,
};


MI_U32 u32AiDevHeapSize = 0;
MI_U32 u32AiChnOutputHeapSize = 0;
MI_U32 u32AoDevHeapSize = 0;

MI_BOOL bAllocAiDevPool = FALSE;
MI_BOOL bAllocAiChnOutputPool = FALSE;
MI_BOOL bAllocAoDevPool = FALSE;

void display_help(void)
{
    printf("----- audio all test -----\n");
    printf("-t : AI/AO run time(s)\n");
	printf("AI Device Id: Amic[0] Dmic[1] I2S RX[2] Linein[3]\n");
    printf("AI test option :\n");
    printf("-I : Enable AI\n");
    printf("-o : AI output Path\n");
    printf("-d : AI Device Id\n");
    printf("-m : AI Sound Mode: Mono[0] Stereo[1] Queue[2]\n");
    printf("-c : AI channel count\n");
    printf("-s : AI Sample Rate\n");
    printf("-v : AI Volume\n");
    printf("-h : AI Enable Hpf\n");
    printf("-g : AI Enable Agc\n");
    printf("-e : AI Enable Eq\n");
    printf("-n : AI Enable Nr\n");
    printf("-r : AI Resample Rate\n");
    printf("-a : AI Aenc Type [g711a g711u g726_16 g726_24 g726_32 g726_40]\n");
    printf("-A : AI Enable Aed\n");
    printf("-b : AI Enable SW Aec\n");
    printf("-B : AI Enable HW Aec\n");
    printf("-S : AI Enable Ssl\n");
    printf("-F : AI Enable Beamforming\n");
    printf("-M : AI mic disttance for Ssl/Bf (cm, step 1 cm)\n");
    printf("-C : AI Bf doc (0~180)\n");
    printf("-w : AI Aec Working Sample Rate(Not necessary)\n");
    printf("-W : AI enable dump pcm data\n");
    printf("\n\n");
	printf("AO Device Id: Lineout[0] I2S TX[1]\n");
    printf("AO test option :\n");
    printf("-O : Enable AO\n");
    printf("-i : AO Input Path\n");
    printf("-D : AO Device Id\n");
    printf("-V : AO Volume\n");
    printf("-H : AO Enable Hpf\n");
    printf("-G : AO Enable Agc\n");
    printf("-E : AO Enable Eq\n");
    printf("-N : AO Enable Nr\n");
    printf("-R : AO Resample Rate\n");
    return;
}

//ES8156 
#define I2C_ADAPTER_STR     ("/dev/i2c-1")
#define ES8156_CHIP_ADDR   (0x08)

typedef enum
{
    EN_CUST_CMD_GET_STATE,
    EN_CUST_CMD_GET_AUDIO_INFO,
    EN_CUST_CMD_CONFIG_I2S,
    EN_CUST_CMD_MAX
}SS_HdmiConv_UsrCmd_e;

typedef enum
{
    EN_I2S_MODE_NORMAL,
    EN_I2S_MODE_LEFT_JUSTIFIED,
    EN_I2S_MODE_RIGHT_JUSTIFIED
}SS_HdmiConv_I2SMode_e;

typedef enum
{
    EN_SIGNAL_NO_CONNECTION,
    EN_SIGNAL_CONNECTED,
    EN_SIGNAL_LOCK
}SS_HdmiConv_SignalInfo_e;
typedef enum
{
    EN_AUDIO_TYPE_PCM,
    EN_AUDIO_TYPE_AC3,
    EN_AUDIO_TYPE_AAC,
    EN_AUDIO_TYPE_DTS
}SS_HdmiConv_AudFormat_e;
typedef struct SS_HdmiConv_AudInfo_s
{
    SS_HdmiConv_AudFormat_e enAudioFormat;
    MI_U8 u8ChannelCount;
    MI_U8 u8BitWidth;     /*16bit, 24bit, 32bit*/
    MI_U32 u32SampleRate; /*48000, 44100, 32000, 16000, 8000*/
}SS_HdmiConv_AudInfo_t;

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
    {0x14, 0xb2},//dac输出gain可以调低 {0x14, 0xbf}
    //{0x14, 0xbf},//dac输出gain可以调低 {0x14, 0xbf}
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
static int s8I2cAdapterFd = -1;
static int bI2cInit = 0;

int I2C_Init(unsigned int timeout, unsigned int retryTime)
{
    int s32Ret;
    if (0 == bI2cInit)
    {
        s8I2cAdapterFd = open((const char *)I2C_ADAPTER_STR, O_RDWR);
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

static int ES8156_SelectWordLength(unsigned char ucharWordLength)
{
    int s32Ret;
    unsigned char val;

    s32Ret = ES8156_ReadByte(0x11, &val);
    if (0 == s32Ret)
    {
        printf("ES8156 reg 0x11 :%x.\n", val);
    }
    else
    {
        return s32Ret;
    }
    switch (ucharWordLength)
    {
        case 16:
        {
            val = (0x3 << 4) | (val & 0xF);
        }
        break;
        case 18:
        {
            val = (0x2 << 4) | (val & 0xF);
        }
        break;
        case 20:
        {
            val = (0x1 << 4) | (val & 0xF);
        }
        break;
        case 24:
        {
            val = val & 0xF;
        }
        break;
        case 32:
        {
            val = (0x4 << 4) | (val & 0xF);
        }
        break;
        default:
            val = (0x3 << 4) | (val & 0xF);
            break;
    }
    s32Ret = ES8156_WriteByte(0x11, val);
    if (0 != s32Ret)
    {
        return s32Ret;
    }
    printf("Write val 0x%x\n", val);

    return 0;

}
static int ES8156_ConfigI2s(SS_HdmiConv_I2SMode_e enI2sMode)
{
    unsigned char u8ReadVal = 0;

    switch (enI2sMode)
    {
        case EN_I2S_MODE_NORMAL:
        {
            ES8156_ReadByte(0x11, &u8ReadVal);
            u8ReadVal &= 0xFE; //bit 0 = 0
            ES8156_WriteByte(0x11, u8ReadVal);
        }
        break;
        case EN_I2S_MODE_LEFT_JUSTIFIED:
        {
            ES8156_ReadByte(0x11, &u8ReadVal);
            u8ReadVal = (u8ReadVal & 0xFE) | 0x1; //bit 0 = 1
            ES8156_WriteByte(0x11, u8ReadVal);
        }
        break;
        case EN_I2S_MODE_RIGHT_JUSTIFIED:
        {
            // I don't know.
        }
        break;
        default:
            break;
    }

    return 0;
}

static int ES8156_Init(void)
{
    int s32Ret;
    unsigned char val = 0;
    unsigned char reg = 0;
    unsigned int u32Index = 0;

    I2C_Init(10, 5);
    s32Ret = ES8156_ReadByte(0xfe, &val);
    if (0 == s32Ret)
    {
        printf("ES8156 ID0:%x.\n", val);
    }
    else
    {
        return s32Ret;
    }

    s32Ret = ES8156_ReadByte(0xfd, &val);
    if (0 == s32Ret)
    {
        printf("ES8156 ID1:%x.\n", val);
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
    I2C_Deinit();

	printf("Deinit ES8156 success.\n");
    return 0;
}
#if 0
void signalHandler(int signo)
{
    switch (signo){
        case SIGALRM:
        case SIGINT:
            printf("Catch signal!!!\n");
            bAiExit = TRUE;
            bAoExit = TRUE;
            break;
   }
   return;
}
#endif
void setTimer(MI_U32 u32RunTime)
{
    struct itimerval new_value, old_value;
    if (0 != u32RunTime)
    {
        new_value.it_value.tv_sec = u32RunTime;
        new_value.it_value.tv_usec = 0;
        new_value.it_interval.tv_sec = 0;
        new_value.it_interval.tv_usec = 0;
        setitimer(ITIMER_REAL, &new_value, &old_value);
    }
    return;
}

MI_BOOL checkParam(void)
{
    MI_BOOL bCheckPass = FALSE;
    do{
        if ((FALSE == bEnableAI) && (FALSE == bEnableAO))
        {
            printf("Neither AI nor AO enabled!!!\n");
            break;
        }

        if (bEnableAI)
        {
            if (NULL == pu8AiOutputPath)
            {
                printf("Ai output path invalid!!!\n");
                break;
            }

            if (AiDevId < 0 || AiDevId > AI_DEV_ID_MAX)
            {
                printf("Ai device id invalid!!!\n");
                break;
            }

			if ((eAiSoundMode != E_AI_SOUND_MODE_MONO)
				&& (eAiSoundMode != E_AI_SOUND_MODE_STEREO)
				&& (eAiSoundMode != E_AI_SOUND_MODE_QUEUE))
			{
				printf("Ai sound mode invalid!!!\n");
				break;
			}

            if (u32AiChnCnt <= 0)
            {
                printf("Ai channel count invalid!!!\n");
                break;
            }

            if (AI_DEV_DMIC == AiDevId)
            {
            	if ((E_AI_SOUND_MODE_MONO == eAiSoundMode)
            		|| (E_AI_SOUND_MODE_QUEUE == eAiSoundMode))
            	{
                	if ((u32AiChnCnt > AI_DMIC_CHN_MAX)
                	)
                	{
                    	printf("Ai channel count invalid!!!\n");
                    	break;
                	}
                }
                else
                {
					if (u32AiChnCnt > AI_DMIC_CHN_MAX / 2)
                	{
                    	printf("Ai channel count invalid!!!\n");
                    	break;
                	}
                }


            }
            else if ((AI_DEV_AMIC == AiDevId)
            			|| (AI_DEV_LineIn == AiDevId)

            		)
            {
            	if ((E_AI_SOUND_MODE_MONO == eAiSoundMode)
            		|| (E_AI_SOUND_MODE_QUEUE == eAiSoundMode))
            	{
	                if (u32AiChnCnt > AI_AMIC_CHN_MAX)
	                {
	                    printf("Ai channel count invalid!!!\n");
	                    break;
	                }
                }
                else
                {
					if (u32AiChnCnt > AI_AMIC_CHN_MAX / 2)
	                {
	                    printf("Ai channel count invalid!!!\n");
	                    break;
	                }
                }
            }
            else
            {
				if ((E_AI_SOUND_MODE_MONO == eAiSoundMode)
					|| (E_AI_SOUND_MODE_QUEUE == eAiSoundMode))
				{
					if ((AI_I2S_NOR_CHN != u32AiChnCnt)
						)
					{
						printf("Ai channel count invalid!!!\n");
	                    break;
					}
				}
				else
				{
					if ((AI_I2S_NOR_CHN / 2 != u32AiChnCnt)
						)
					{
						printf("Ai channel count invalid!!!\n");
	                    break;
					}
				}
            }

            if (
                    (E_MI_AUDIO_SAMPLE_RATE_8000 != eAiSampleRate)
                &&  (E_MI_AUDIO_SAMPLE_RATE_16000 != eAiSampleRate)
                &&  (E_MI_AUDIO_SAMPLE_RATE_32000 != eAiSampleRate)
                &&  (E_MI_AUDIO_SAMPLE_RATE_48000 != eAiSampleRate)
            )
            {
                printf("AI sample rate invalid!!!\n");
                break;
            }

            if (bAiEnableAec)
            {
                if (
                	((E_MI_AUDIO_SAMPLE_RATE_8000 != eAiSampleRate) && (E_MI_AUDIO_SAMPLE_RATE_16000 != eAiSampleRate))
                    &&
                    ((E_MI_AUDIO_SAMPLE_RATE_INVALID == u32VqeWorkingSampleRate)
                		|| ((E_MI_AUDIO_SAMPLE_RATE_8000 != u32VqeWorkingSampleRate)
                			&& (E_MI_AUDIO_SAMPLE_RATE_16000 != u32VqeWorkingSampleRate)))
                	)
                {
                    printf("Aec only support 8K/16K!!!\n");
                    break;
                }

                if (AI_DEV_I2S_RX == AiDevId)
                {
                    printf("I2S RX not support AEC!!!\n");
                    break;
                }
            }

			if (TRUE == bAiEnableHwAec)
			{
				if ((AI_DEV_AMIC != AiDevId) && (AI_DEV_LineIn != AiDevId))
				{
					printf("Hw Aec only support Amic/Linein!!!\n");
					break;
				}
			}

			if (TRUE == bAiEnableHwAec)
			{
				if (((E_AI_SOUND_MODE_MONO != eAiSoundMode) || (2 != u32AiChnCnt)))
				{
					printf("Hw Aec only support Mono mode 2 Channel!!!\n");
					break;
				}
			}

            if (bAiEnableVqe)
            {
                if (
                	((E_MI_AUDIO_SAMPLE_RATE_8000 != eAiSampleRate)
                		&& (E_MI_AUDIO_SAMPLE_RATE_16000 != eAiSampleRate)
                    	&& (E_MI_AUDIO_SAMPLE_RATE_48000 != eAiSampleRate))
                    &&
                    ((E_MI_AUDIO_SAMPLE_RATE_INVALID == u32VqeWorkingSampleRate)
                		|| ((E_MI_AUDIO_SAMPLE_RATE_8000 != u32VqeWorkingSampleRate)
                			&& (E_MI_AUDIO_SAMPLE_RATE_16000 != u32VqeWorkingSampleRate)
                			&& (E_MI_AUDIO_SAMPLE_RATE_48000 != u32VqeWorkingSampleRate))))
                {
                    printf("Vqe only support 8K/16/48K!!!\n");
                    break;
                }

                if (bAiEnableHpf)
                {
                    if (((E_MI_AUDIO_SAMPLE_RATE_48000 == eAiSampleRate)
                    	&& (E_MI_AUDIO_SAMPLE_RATE_INVALID == u32VqeWorkingSampleRate))
                    		|| (E_MI_AUDIO_SAMPLE_RATE_48000 == u32VqeWorkingSampleRate))
                    {

                        printf("Hpf not support 48K!!!\n");
                        break;
                    }
                }
            }

            if (TRUE == bAiEnableResample)
            {
                if (
                        (E_MI_AUDIO_SAMPLE_RATE_8000 != eAiOutputResampleRate)
                    &&  (E_MI_AUDIO_SAMPLE_RATE_16000 != eAiOutputResampleRate)
                    &&  (E_MI_AUDIO_SAMPLE_RATE_32000 != eAiOutputResampleRate)
                    &&  (E_MI_AUDIO_SAMPLE_RATE_48000 != eAiOutputResampleRate)
                )
                {
                    printf("Ai resample rate invalid!!!\n");
                    break;
                }

                if (E_MI_AUDIO_SAMPLE_RATE_INVALID == u32VqeWorkingSampleRate)
                {
                    if (eAiOutputResampleRate == eAiSampleRate)
                    {
                        printf("eAiSampleRate:%d eAiOutputResampleRate:%d. It does not need to resample.\n",
                                eAiSampleRate, eAiOutputResampleRate);
                        break;
                    }
                }
                else
                {
                    if (eAiOutputResampleRate == u32VqeWorkingSampleRate)
                    {
                        printf("u32VqeWorkingSampleRate:%d eAiOutputResampleRate:%d. It does not need to resample.\n",
                                u32VqeWorkingSampleRate, eAiOutputResampleRate);
                        break;
                    }
                }
            }

            if (TRUE == bAiEnableAenc)
            {
                if (
                        (E_MI_AUDIO_AENC_TYPE_G711A != eAiAencType)
                    &&  (E_MI_AUDIO_AENC_TYPE_G711U != eAiAencType)
                    &&  (E_MI_AUDIO_AENC_TYPE_G726 != eAiAencType)
                )
                {
                    printf("Ai aenc type invalid!!!\n");
                    break;
                }

                if (E_MI_AUDIO_AENC_TYPE_G726 == eAiAencType)
                {
                    if (
                            (E_MI_AUDIO_G726_MODE_16 != eAiAencG726Mode)
                        &&  (E_MI_AUDIO_G726_MODE_24 != eAiAencG726Mode)
                        &&  (E_MI_AUDIO_G726_MODE_32 != eAiAencG726Mode)
                        &&  (E_MI_AUDIO_G726_MODE_40 != eAiAencG726Mode)
                    )
                    {
                        printf("Ai G726 mode invalid!!!\n");
                        break;
                    }
                }

                if (E_MI_AUDIO_AENC_TYPE_G726 == eAiAencType)
                {
                    if (E_AI_SOUND_MODE_STEREO == eAiSoundMode)
                    {
                        printf("Wav not support stereo g726!!!\n");
                        break;
                    }
                }
            }

            if (bAiEnableSsl || bAiEnableBf)
            {
				if (0 == u32MicDistance)
				{
					printf("Ai mic distance invalid!!!\n");
					break;
				}

				if (E_AI_SOUND_MODE_STEREO != eAiSoundMode)
				{
					printf("Ssl/Bf only support stereo mode!!!\n");
					break;
				}

				if (bAiEnableBf && bAiSetBfDoa)
				{
                    if (0 < s32AiBfDoa || s32AiBfDoa > 180)
                    {
                        printf("s32AiBfDoa only supports 0~180 currently.\n");
                        break;
                    }
				}
            }

            if (TRUE == bAiSetVolume)
            {
                if (AI_DEV_AMIC == AiDevId)
                {
                    if ((s32AiVolume < AI_VOLUME_AMIC_MIN) || (s32AiVolume > AI_VOLUME_AMIC_MAX))
                    {
                        printf("Ai volume invalid!!!\n");
                        break;
                    }
                }
                else if (AI_DEV_DMIC == AiDevId)
                {
                    if ((s32AiVolume < AI_VOLUME_DMIC_MIN) || (s32AiVolume > AI_VOLUME_DMIC_MAX))
                    {
                        printf("Ai volume invalid!!!\n");
                        break;
                    }
                }

                else if (AI_DEV_LineIn == AiDevId)
                {
                    if ((s32AiVolume < AI_VOLUME_AMIC_MIN) || (s32AiVolume > AI_VOLUME_LINEIN_MAX))
                    {
                        printf("Ai volume invalid!!!\n");
                        break;
                    }
                }

                else if (AI_DEV_I2S_RX == AiDevId)
                {
                    printf("I2S RX is not supported volume setting!!!\n");
                    break;
                }
            }
        }

        if (bEnableAO)
        {
            if (NULL == pu8AoInputPath)
            {
                printf("AO input path invalid!!!\n");
                break;
            }

            if ((AoDevId < 0) || (AoDevId > AO_DEV_ID_MAX))
            {
                printf("Ao device id invalid!!!\n");
                break;
            }

            if (TRUE == bAoEnableResample)
            {
                if (
                        (E_MI_AUDIO_SAMPLE_RATE_8000 != eAoOutSampleRate)
                    &&  (E_MI_AUDIO_SAMPLE_RATE_16000 != eAoOutSampleRate)
                    &&  (E_MI_AUDIO_SAMPLE_RATE_32000 != eAoOutSampleRate)
                    &&  (E_MI_AUDIO_SAMPLE_RATE_48000 != eAoOutSampleRate)
                )
                {
                    printf("AO resample rate invalid!!!\n");
                    break;
                }
            }

            if (TRUE == bAoSetVolume)
            {
                if ((s32AoVolume < AO_VOLUME_MIN) || (s32AoVolume > AO_VOLUME_MAX))
                {
                    printf("AO Volume invalid!!!\n");
                    break;
                }
            }

            if ( NULL == strstr((const char*)pu8AoInputPath, ".wav") )
            {
                printf("Only support wav file.\n");
                break;
            }
        }
        bCheckPass = TRUE;
    }while(0);
    return bCheckPass;
}

void initParam(void)
{
    eAiWavSampleRate = eAiSampleRate;

    if (E_MI_AUDIO_AENC_TYPE_G711A == eAiAencType)
    {
        eAiWavAencType = E_AENC_TYPE_G711A;
    }
    else if (E_MI_AUDIO_AENC_TYPE_G711U == eAiAencType)
    {
        eAiWavAencType = E_AENC_TYPE_G711U;
    }
    else if ((E_MI_AUDIO_AENC_TYPE_G726 == eAiAencType) && (E_MI_AUDIO_G726_MODE_16 == eAiAencG726Mode))
    {
        eAiWavAencType = E_AENC_TYPE_G726_16;
    }
    else if ((E_MI_AUDIO_AENC_TYPE_G726 == eAiAencType) && (E_MI_AUDIO_G726_MODE_24 == eAiAencG726Mode))
    {
        eAiWavAencType = E_AENC_TYPE_G726_24;
    }
    else if ((E_MI_AUDIO_AENC_TYPE_G726 == eAiAencType) && (E_MI_AUDIO_G726_MODE_32 == eAiAencG726Mode))
    {
        eAiWavAencType = E_AENC_TYPE_G726_32;
    }
    else if ((E_MI_AUDIO_AENC_TYPE_G726 == eAiAencType) && (E_MI_AUDIO_G726_MODE_40 == eAiAencG726Mode))
    {
        eAiWavAencType = E_AENC_TYPE_G726_40;
    }
    else
    {
        eAiWavAencType = PCM;
    }

	if (AI_DEV_I2S_RX == AiDevId)
	{
		if (
			(((E_AI_SOUND_MODE_MONO == eAiSoundMode) || (E_AI_SOUND_MODE_QUEUE == eAiSoundMode)) && (AI_I2S_NOR_CHN == u32AiChnCnt))
			|| ((E_AI_SOUND_MODE_STEREO == eAiSoundMode) && (AI_I2S_NOR_CHN / 2 == u32AiChnCnt)))
		{
			bAiI2sNormalMode = TRUE;
		}
		else
		{
			bAiI2sNormalMode = FALSE;
		}
	}

	if (bAiEnableSsl || bAiEnableBf)
	{
		stSslInit.u32MicDistance = u32MicDistance;
		stBfInit.u32MicDistance = u32MicDistance;
	}

    //signal(SIGALRM, signalHandler);
    //signal(SIGINT, signalHandler);

    return;
}

MI_S32 allocPrivatePool(void)
{
	MI_SYS_GlobalPrivPoolConfig_t stGlobalPrivPoolConf;
	MI_U32 u32HeapSize = 0;
	MI_U32 u32ChnCnt = 0;
	MI_U32 u32PtNum = 0;
	MI_U32 u32BufSize;
	MI_U32 u32BitWidth = 2;
	MI_U32 u32AlignSize = 4 * 1024;
	MI_U32 u32ChnIdx;

	if (bEnableAI)
	{
		u32ChnCnt = u32AiChnCnt;
		if (E_MI_AUDIO_SOUND_MODE_STEREO == eAiSoundMode)
		{
			u32ChnCnt *= 2;
		}

		u32HeapSize = u32BitWidth * u32ChnCnt * 2 * eAiSampleRate;

		u32HeapSize += (MIU_WORD_BYTE_SIZE - (u32HeapSize % MIU_WORD_BYTE_SIZE));

        if (u32HeapSize >= AI_DMA_BUFFER_MAX_SIZE)
        {
			u32HeapSize = AI_DMA_BUFFER_MAX_SIZE;
        }
        else if (u32HeapSize >= AI_DMA_BUFFER_MID_SIZE)
        {
			u32HeapSize = AI_DMA_BUFFER_MID_SIZE;
        }
		else
        {
			u32HeapSize = AI_DMA_BUFFER_MIN_SIZE;
        }

		u32AiDevHeapSize = u32HeapSize;

		memset(&stGlobalPrivPoolConf, 0x0, sizeof(stGlobalPrivPoolConf));
		stGlobalPrivPoolConf.bCreate = TRUE;
		stGlobalPrivPoolConf.eConfigType = E_MI_SYS_PER_DEV_PRIVATE_POOL;
		stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.eModule = E_MI_MODULE_ID_AI;
		stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.u32Devid = AiDevId;
		stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.u32PrivateHeapSize = u32HeapSize;
		ExecFunc(MI_SYS_ConfigPrivateMMAPool(&stGlobalPrivPoolConf), MI_SUCCESS);

		bAllocAiDevPool = TRUE;

		u32PtNum = eAiSampleRate / 16;
		u32BufSize = u32PtNum * u32BitWidth;	// for one date channel

		if (E_MI_AUDIO_SOUND_MODE_STEREO == eAiSoundMode)
		{
			u32BufSize *= 2;
		}

		if (AI_DEV_I2S_RX != AiDevId)
		{
			u32BufSize *= 2;
		}

		// 4K alignment
		if (0 != (u32BufSize % u32AlignSize))
		{
			u32BufSize = ((u32BufSize / u32AlignSize) + 1) * u32AlignSize;
		}

		u32HeapSize = u32BufSize * TOTAL_BUF_DEPTH;
		u32AiChnOutputHeapSize = u32HeapSize;

		for (u32ChnIdx = 0; u32ChnIdx < u32AiChnCnt; u32ChnIdx++)
		{
			memset(&stGlobalPrivPoolConf, 0x0, sizeof(stGlobalPrivPoolConf));
			stGlobalPrivPoolConf.bCreate = TRUE;
			stGlobalPrivPoolConf.eConfigType = E_MI_SYS_PER_CHN_PORT_OUTPUT_POOL;
			stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.eModule = E_MI_MODULE_ID_AI;
			stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32Channel = u32ChnIdx;
			stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32Devid = AiDevId;
			stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32Port = 0;
			stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32PrivateHeapSize = u32HeapSize;
			ExecFunc(MI_SYS_ConfigPrivateMMAPool(&stGlobalPrivPoolConf), MI_SUCCESS);
		}
		bAllocAiChnOutputPool = TRUE;
	}

	if (bEnableAO)
	{
		// I can not get sampling rate of ao device, so use max sample rate to alloc heap
		u32HeapSize = u32BitWidth * E_MI_AUDIO_SAMPLE_RATE_48000 / 2;
		u32HeapSize += (MIU_WORD_BYTE_SIZE - (u32HeapSize % MIU_WORD_BYTE_SIZE));
		if (u32HeapSize >= AO_DMA_BUFFER_MAX_SIZE)
		{
			u32HeapSize = AO_DMA_BUFFER_MAX_SIZE;
		}
		else if (u32HeapSize >= AO_DMA_BUFFER_MID_SIZE)
		{
			u32HeapSize = AO_DMA_BUFFER_MID_SIZE;
		}
		else
		{
			u32HeapSize = AO_DMA_BUFFER_MIN_SIZE;
		}

		// 2 buf, hw buffer + copy buffer
		u32HeapSize *= 2;
		u32AoDevHeapSize = u32HeapSize;
		memset(&stGlobalPrivPoolConf, 0x0, sizeof(stGlobalPrivPoolConf));
		stGlobalPrivPoolConf.bCreate = TRUE;
		stGlobalPrivPoolConf.eConfigType = E_MI_SYS_PER_DEV_PRIVATE_POOL;
		stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.eModule = E_MI_MODULE_ID_AO;
		stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.u32Devid = AoDevId;
		stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.u32PrivateHeapSize = u32HeapSize;
		ExecFunc(MI_SYS_ConfigPrivateMMAPool(&stGlobalPrivPoolConf), MI_SUCCESS);
		bAllocAoDevPool = TRUE;
	}
	return MI_SUCCESS;
}

MI_S32 freePrivatePool(void)
{
	MI_SYS_GlobalPrivPoolConfig_t stGlobalPrivPoolConf;
	MI_U32 u32ChnIdx;

	if (bEnableAI)
	{
		if (TRUE == bAllocAiDevPool)
		{
			memset(&stGlobalPrivPoolConf, 0x0, sizeof(stGlobalPrivPoolConf));
			stGlobalPrivPoolConf.bCreate = FALSE;
			stGlobalPrivPoolConf.eConfigType = E_MI_SYS_PER_DEV_PRIVATE_POOL;
			stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.eModule = E_MI_MODULE_ID_AI;
			stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.u32Devid = AiDevId;
			stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.u32PrivateHeapSize = u32AiDevHeapSize;
			ExecFunc(MI_SYS_ConfigPrivateMMAPool(&stGlobalPrivPoolConf), MI_SUCCESS);

			bAllocAiDevPool = FALSE;
		}

		if (TRUE == bAllocAiChnOutputPool)
		{
			for (u32ChnIdx = 0; u32ChnIdx < u32AiChnCnt; u32ChnIdx++)
			{
				memset(&stGlobalPrivPoolConf, 0x0, sizeof(stGlobalPrivPoolConf));
				stGlobalPrivPoolConf.bCreate = FALSE;
				stGlobalPrivPoolConf.eConfigType = E_MI_SYS_PER_CHN_PORT_OUTPUT_POOL;
				stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.eModule = E_MI_MODULE_ID_AI;
				stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32Channel = u32ChnIdx;
				stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32Devid = AiDevId;
				stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32Port = 0;
				stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32PrivateHeapSize = u32AiChnOutputHeapSize;
				ExecFunc(MI_SYS_ConfigPrivateMMAPool(&stGlobalPrivPoolConf), MI_SUCCESS);
			}
			bAllocAiChnOutputPool = FALSE;
		}
	}

	if (bEnableAO)
	{
		if (TRUE == bAllocAoDevPool)
		{
			memset(&stGlobalPrivPoolConf, 0x0, sizeof(stGlobalPrivPoolConf));
			stGlobalPrivPoolConf.bCreate = FALSE;
			stGlobalPrivPoolConf.eConfigType = E_MI_SYS_PER_DEV_PRIVATE_POOL;
			stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.eModule = E_MI_MODULE_ID_AO;
			stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.u32Devid = AoDevId;
			stGlobalPrivPoolConf.uConfig.stPreDevPrivPoolConfig.u32PrivateHeapSize = u32AoDevHeapSize;
			ExecFunc(MI_SYS_ConfigPrivateMMAPool(&stGlobalPrivPoolConf), MI_SUCCESS);
			bAllocAoDevPool = FALSE;
		}
	}
	return MI_SUCCESS;
}

void freePrivatePoolExit(void)
{
	MI_S32 s32Ret = MI_SUCCESS;
	s32Ret = freePrivatePool();
	if (MI_SUCCESS != s32Ret)
	{
		printf("Failed to free private pool!!!\n");
	}
	return;
}

#if 1
MI_S32 initAi(void)
{
    MI_AUDIO_Attr_t     stAiSetAttr;
    MI_AUDIO_Attr_t     stAiGetAttr;
    AiOutFileName_t     stAiFileName;
    MI_AI_AencConfig_t  stAiSetAencConfig, stAiGetAencConfig;
    MI_AI_VqeConfig_t   stAiSetVqeConfig, stAiGetVqeConfig;
    MI_AI_AedConfig_t   stAiSetAedConfig, stAiGetAedConfig;
    MI_U32              u32ChnIdx;
    MI_U32              u32ChnCnt;
    MI_S8               s8OutputFileName[512];
    MI_S8               s8SrcPcmOutputFileName[512];
    MI_S32              s32Ret;
    WaveFileHeader_t    stAiWavHead;
    MI_SYS_ChnPort_t    stAiChnOutputPort0[MI_AUDIO_MAX_CHN_NUM];
    MI_AI_ChnParam_t    stAiChnParam;
    MI_AI_SslInitAttr_t	stAiGetSslInitAttr;
    MI_AI_SslConfigAttr_t	stAiGetSslConfigAttr;
    MI_AI_BfInitAttr_t		stAiGetBfInitAttr;
    MI_AI_BfConfigAttr_t	stAiGetBfConfigAttr;

    memset(&stAiSetAttr, 0x0, sizeof(MI_AUDIO_Attr_t));
    memset(&stAiGetAttr, 0x0, sizeof(MI_AUDIO_Attr_t));
    stAiSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stAiSetAttr.eSamplerate = eAiSampleRate;   //48000
    stAiSetAttr.eSoundmode = eAiSoundMode;     //1
    stAiSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_SLAVE; //E_MI_AUDIO_MODE_I2S_MASTER;  //1 I2S  Slave
    stAiSetAttr.u32ChnCnt = u32AiChnCnt;       //1 1;//
    stAiSetAttr.u32CodecChnCnt = 0; // useless
    stAiSetAttr.u32FrmNum = 6;  // useless
    stAiSetAttr.u32PtNumPerFrm = stAiSetAttr.eSamplerate / 32; //stAiSetAttr.eSamplerate / 16; // for aec    //32
    stAiSetAttr.WorkModeSetting.stI2sConfig.bSyncClock = TRUE; //FALSE; // useless   //TRUE //
    stAiSetAttr.WorkModeSetting.stI2sConfig.eFmt = E_MI_AUDIO_I2S_FMT_I2S_MSB;
    stAiSetAttr.WorkModeSetting.stI2sConfig.eMclk = E_MI_AUDIO_I2S_MCLK_0;

    printf("func(%s),line[%d],AiDevId=%d,eBitwidth=%d,eSamplerate=%d,eSoundmode=%d,eWorkmode=%d,u32AiChnCnt=%d\n",__func__,__LINE__,AiDevId,stAiSetAttr.eBitwidth,stAiSetAttr.eSamplerate,stAiSetAttr.eSoundmode,stAiSetAttr.eWorkmode,stAiSetAttr.u32ChnCnt);

    if ((E_AI_SOUND_MODE_MONO == eAiSoundMode)
		|| (E_AI_SOUND_MODE_QUEUE == eAiSoundMode))
	{
		eAiWavSoundMode = E_SOUND_MODE_MONO;
	}
	else
	{
		eAiWavSoundMode = E_SOUND_MODE_STEREO;
	}

	if (bAiEnableBf)
	{
		eAiWavSoundMode = E_SOUND_MODE_MONO;
	}

    memset(&stAiSetVqeConfig, 0x0, sizeof(MI_AI_VqeConfig_t));
    if (bAiEnableVqe)
    {
		if ((E_AI_SOUND_MODE_MONO == eAiSoundMode)
			|| (E_AI_SOUND_MODE_QUEUE == eAiSoundMode))
		{
			stAiSetVqeConfig.u32ChnNum = 1;
		}
		else
		{
			stAiSetVqeConfig.u32ChnNum = 2;
		}

		if (bAiEnableBf)
		{
			stAiSetVqeConfig.u32ChnNum = 1;
		}

        stAiSetVqeConfig.bAecOpen = bAiEnableAec;
        stAiSetVqeConfig.bAgcOpen = bAiEnableAgc;
        stAiSetVqeConfig.bAnrOpen = bAiEnableNr;
        stAiSetVqeConfig.bEqOpen = bAiEnableEq;
        stAiSetVqeConfig.bHpfOpen = bAiEnableHpf;
        stAiSetVqeConfig.s32FrameSample = 128;
        if ((E_MI_AUDIO_SAMPLE_RATE_INVALID != u32VqeWorkingSampleRate))
        {
        	stAiSetVqeConfig.s32WorkSampleRate = u32VqeWorkingSampleRate;
        }
        else
        {
			stAiSetVqeConfig.s32WorkSampleRate = eAiSampleRate;
        }

        // AEC
        memcpy(&stAiSetVqeConfig.stAecCfg, &stAecCfg, sizeof(MI_AI_AecConfig_t));

        // AGC
        memcpy(&stAiSetVqeConfig.stAgcCfg, &stAgcCfg, sizeof(MI_AUDIO_AgcConfig_t));

        // ANR
        memcpy(&stAiSetVqeConfig.stAnrCfg, &stAnrCfg, sizeof(MI_AUDIO_AnrConfig_t));

        // EQ
        memcpy(&stAiSetVqeConfig.stEqCfg, &stEqCfg, sizeof(MI_AUDIO_EqConfig_t));

        // HPF
        memcpy(&stAiSetVqeConfig.stHpfCfg, &stHpfCfg, sizeof(MI_AUDIO_HpfConfig_t));
    }

    memset(&stAiSetAencConfig, 0x0, sizeof(MI_AI_AencConfig_t));
    if (bAiEnableAenc)
    {
        stAiSetAencConfig.eAencType = eAiAencType;
        if (E_MI_AUDIO_AENC_TYPE_G726 == eAiAencType)
        {
            stAiSetAencConfig.stAencG726Cfg.eG726Mode = eAiAencG726Mode;
        }
    }

    memset(&stAiFileName, 0x0, sizeof(AiOutFileName_t));
    stAiFileName.AiDevId = AiDevId;
    stAiFileName.bEnableAenc = bAiEnableAenc;
    stAiFileName.bEnableRes = bAiEnableResample;
    stAiFileName.eOutSampleRate = eAiOutputResampleRate;
    stAiFileName.ps8OutputFile = (MI_S8*)s8OutputFileName;
    stAiFileName.ps8SrcPcmOutputFile = (MI_S8*)s8SrcPcmOutputFileName;
    stAiFileName.ps8OutputPath = (MI_S8*)pu8AiOutputPath;
    memcpy(&stAiFileName.stAiAttr, &stAiSetAttr, sizeof(MI_AUDIO_Attr_t));
    memcpy(&stAiFileName.stSetVqeConfig, &stAiSetVqeConfig, sizeof(MI_AI_VqeConfig_t));
    memcpy(&stAiFileName.stSetAencConfig, &stAiSetAencConfig, sizeof(MI_AI_AencConfig_t));

	if ((E_MI_AUDIO_SAMPLE_RATE_INVALID != u32VqeWorkingSampleRate))
    {
		stAiFileName.bSetVqeWorkingSampleRate = TRUE;
		stAiFileName.eVqeWorkingSampleRate = u32VqeWorkingSampleRate;
    }

    if (TRUE == bAiEnableBf)
    {
        stAiFileName.bEnableBf = TRUE;
    }

    memset(&stAiWavHead, 0x0, sizeof(WaveFileHeader_t));

    if (bAiEnableHwAec)
    {
        u32ChnCnt = 1;
    }
    else
    {
        u32ChnCnt = u32AiChnCnt;
    }
    
    ExecFuncNoExit(MI_AI_SetPubAttr(AiDevId, &stAiSetAttr), MI_SUCCESS, s32Ret);
    if (MI_SUCCESS != s32Ret)
    {
		goto ERROR_RETURN;
    }

    ExecFuncNoExit(MI_AI_GetPubAttr(AiDevId, &stAiGetAttr), MI_SUCCESS, s32Ret);
    if (MI_SUCCESS != s32Ret)
    {
		goto ERROR_RETURN;
    }

    ExecFuncNoExit(MI_AI_Enable(AiDevId), MI_SUCCESS, s32Ret);
    if (MI_SUCCESS != s32Ret)
    {
		goto ERROR_RETURN;
    }

    if (bAiSetVolume)
    {
        for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
        {
            ExecFunc(MI_AI_SetVqeVolume(AiDevId, u32ChnIdx, s32AiVolume), MI_SUCCESS);
        }
    }

    if (bAiEnableHwAec)
    {
        ExecFunc(MI_AI_SetVqeVolume(AiDevId, 1, 1), MI_SUCCESS);
    }

    memset(&stAiChnPriv, 0x0, sizeof(stAiChnPriv));
    memset(&stAiChnOutputPort0, 0x0, sizeof(stAiChnOutputPort0));

    if ((E_AI_SOUND_MODE_QUEUE == eAiSoundMode) || bAiEnableHwAec)
    {
        u32ChnCnt = 1;
    }
    else
    {
        u32ChnCnt = u32AiChnCnt;
    }

    for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
    {
        stAiChnPriv[u32ChnIdx].AiChn = u32ChnIdx;
        stAiChnPriv[u32ChnIdx].AiDevId = AiDevId;
        stAiChnPriv[u32ChnIdx].s32Fd = AiChnFd[u32ChnIdx];
        stAiChnPriv[u32ChnIdx].s32SrcPcmFd = AiChnSrcPcmFd[u32ChnIdx];
        stAiChnPriv[u32ChnIdx].u32ChnCnt = u32AiChnCnt;
        stAiChnPriv[u32ChnIdx].u32TotalSize = 0;
        stAiChnPriv[u32ChnIdx].u32SrcPcmTotalSize = 0;

        stAiChnOutputPort0[u32ChnIdx].eModId = E_MI_MODULE_ID_AI;
        stAiChnOutputPort0[u32ChnIdx].u32DevId = AiDevId;
        stAiChnOutputPort0[u32ChnIdx].u32ChnId = u32ChnIdx;
        stAiChnOutputPort0[u32ChnIdx].u32PortId = 0;
        //ExecFunc(MI_SYS_SetChnOutputPortDepth(&stAiChnOutputPort0[u32ChnIdx], 1, TOTAL_BUF_DEPTH), MI_SUCCESS);
        ExecFunc(MI_SYS_SetChnOutputPortDepth(&stAiChnOutputPort0[u32ChnIdx], 8, TOTAL_BUF_DEPTH), MI_SUCCESS);
    }

    for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
    {
        if (bAiEnableHwAec)
        {
            ExecFunc(MI_AI_SetExtAecChn(AiDevId, u32ChnIdx, 1), MI_SUCCESS);
        }
        ExecFuncNoExit(MI_AI_EnableChn(AiDevId, u32ChnIdx), MI_SUCCESS, s32Ret);
        if (MI_SUCCESS != s32Ret)
        {
			goto DISABLE_DEVICE;
        }
    }

    for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
    {
        // AED
        if (bAiEnableAed)
        {
            memcpy(&stAiSetAedConfig, &stAedCfg, sizeof(MI_AI_AedConfig_t));
            ExecFunc(MI_AI_SetAedAttr(AiDevId, u32ChnIdx, &stAiSetAedConfig), MI_SUCCESS);
            ExecFunc(MI_AI_GetAedAttr(AiDevId, u32ChnIdx, &stAiGetAedConfig), MI_SUCCESS);
            ExecFunc(MI_AI_EnableAed(AiDevId, u32ChnIdx), MI_SUCCESS);
            stAiChnPriv[u32ChnIdx].bEnableAed = TRUE;
        }

		// SSL
		if (bAiEnableSsl)
		{
			if ((TRUE == bAiEnableBf) && (FALSE == bAiSetBfDoa))
			{
				stSslInit.bBfMode = TRUE;
			}

			if (E_MI_AUDIO_SAMPLE_RATE_8000 == eAiSampleRate)
			{
				stSslConfig.s32DirectionFrameNum = 50;
			}
			else if (E_MI_AUDIO_SAMPLE_RATE_16000 == eAiSampleRate)
			{
				stSslConfig.s32DirectionFrameNum = 100;
			}
			else if (E_MI_AUDIO_SAMPLE_RATE_32000 == eAiSampleRate)
			{
				stSslConfig.s32DirectionFrameNum = 200;
			}
			else if (E_MI_AUDIO_SAMPLE_RATE_48000 == eAiSampleRate)
			{
				stSslConfig.s32DirectionFrameNum = 300;
			}

			ExecFunc(MI_AI_SetSslInitAttr(AiDevId, u32ChnIdx, &stSslInit), MI_SUCCESS);
			ExecFunc(MI_AI_GetSslInitAttr(AiDevId, u32ChnIdx, &stAiGetSslInitAttr), MI_SUCCESS);
			ExecFunc(MI_AI_SetSslConfigAttr(AiDevId, u32ChnIdx, &stSslConfig), MI_SUCCESS);
			ExecFunc(MI_AI_GetSslConfigAttr(AiDevId, u32ChnIdx, &stAiGetSslConfigAttr), MI_SUCCESS);
			ExecFunc(MI_AI_EnableSsl(AiDevId, u32ChnIdx), MI_SUCCESS);
		}

		// BF
		if (bAiEnableBf)
		{
			ExecFunc(MI_AI_SetBfInitAttr(AiDevId, u32ChnIdx, &stBfInit), MI_SUCCESS);
			ExecFunc(MI_AI_GetBfInitAttr(AiDevId, u32ChnIdx, &stAiGetBfInitAttr), MI_SUCCESS);
			ExecFunc(MI_AI_SetBfConfigAttr(AiDevId, u32ChnIdx, &stBfConfig), MI_SUCCESS);
			ExecFunc(MI_AI_GetBfConfigAttr(AiDevId, u32ChnIdx, &stAiGetBfConfigAttr), MI_SUCCESS);
			if (bAiSetBfDoa)
			{
			    ExecFunc(MI_AI_SetBfAngle(AiDevId, u32ChnIdx, s32AiBfDoa), MI_SUCCESS);
			}
			ExecFunc(MI_AI_EnableBf(AiDevId, u32ChnIdx), MI_SUCCESS);
		}

        // VQE
        if(bAiEnableVqe)
        {
            ExecFunc(MI_AI_SetVqeAttr(AiDevId, u32ChnIdx, 0, 0, &stAiSetVqeConfig), MI_SUCCESS);
            ExecFunc(MI_AI_GetVqeAttr(AiDevId, u32ChnIdx, &stAiGetVqeConfig), MI_SUCCESS);
            ExecFunc(MI_AI_EnableVqe(AiDevId, u32ChnIdx), MI_SUCCESS);
            if ((E_MI_AUDIO_SAMPLE_RATE_INVALID != u32VqeWorkingSampleRate))
            {
            	eAiWavSampleRate = u32VqeWorkingSampleRate;
            }
        }

        // RES
        if(bAiEnableResample)
        {
            ExecFunc(MI_AI_EnableReSmp(AiDevId, u32ChnIdx, eAiOutputResampleRate), MI_SUCCESS);
            eAiWavSampleRate = eAiOutputResampleRate;
        }

        // AENC
        if (bAiEnableAenc)
        {
            ExecFunc(MI_AI_SetAencAttr(AiDevId, u32ChnIdx, &stAiSetAencConfig), MI_SUCCESS);
            ExecFunc(MI_AI_GetAencAttr(AiDevId, u32ChnIdx, &stAiGetAencConfig), MI_SUCCESS);
            ExecFunc(MI_AI_EnableAenc(AiDevId, u32ChnIdx), MI_SUCCESS);
        }
        
    }
    printf("create ai thread.\n");
    return 0;

DISABLE_CHANNEL:
	for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
	{
		ExecFunc(MI_AI_DisableChn(AiDevId, u32ChnIdx), MI_SUCCESS);
	}
DISABLE_DEVICE:
	ExecFunc(MI_AI_Disable(AiDevId), MI_SUCCESS);

ERROR_RETURN:
	return s32Ret;
}
#endif

MI_S32 deinitAi(void)
{
    MI_U32 u32ChnIdx;
    MI_U32 u32ChnCnt;

    if ((E_AI_SOUND_MODE_QUEUE == eAiSoundMode) || bAiEnableHwAec)
    {
        u32ChnCnt = 1;
    }
    else
    {
        u32ChnCnt = u32AiChnCnt;
    }

    for (u32ChnIdx = 0; u32ChnIdx < u32ChnCnt; u32ChnIdx++)
    {
        if(bAiEnableAed)
        {
            ExecFunc(MI_AI_DisableAed(AiDevId, u32ChnIdx), MI_SUCCESS);
        }

        if (bAiEnableAenc)
        {
            ExecFunc(MI_AI_DisableAenc(AiDevId, u32ChnIdx), MI_SUCCESS);
        }

        if(bAiEnableResample)
        {
            ExecFunc(MI_AI_DisableReSmp(AiDevId, u32ChnIdx), MI_SUCCESS);
        }

        if(bAiEnableVqe)
        {
            ExecFunc(MI_AI_DisableVqe(AiDevId, u32ChnIdx), MI_SUCCESS);
        }

		if (bAiEnableSsl)
		{
			ExecFunc(MI_AI_DisableSsl(AiDevId, u32ChnIdx), MI_SUCCESS);
		}

		if (bAiEnableBf)
		{
			ExecFunc(MI_AI_DisableBf(AiDevId, u32ChnIdx), MI_SUCCESS);
		}

        ExecFunc(MI_AI_DisableChn(AiDevId, u32ChnIdx), MI_SUCCESS);
    }
    ExecFunc(MI_AI_Disable(AiDevId), MI_SUCCESS);
    return 0;
}

void SetAudioMode(void)
{
    MI_BOOL     bCheckPass = FALSE;

     // enable Ai
    bEnableAI = TRUE;

    // set Ai output path
    pu8AiOutputPath = "/mnt";

    // set Ai Aenc Type
    bAiEnableAenc = TRUE;
    eAiAencType = E_MI_AUDIO_AENC_TYPE_G711A;

    // set Ai chn num
    u32AiChnCnt = 1;
    printf("u32AiChnCnt : %d \n", u32AiChnCnt);

    // set Ai volume
    s32AiVolume = 0;//(MI_S32)atoi(optarg);
    bAiSetVolume = TRUE;

    // set Ai device ID
    AiDevId = 0;//atoi(optarg);
    printf("AiDevId : %d \n", AiDevId);

    // set Ai sample rate
    eAiSampleRate = 48000;//(MI_AUDIO_SampleRate_e)atoi(optarg);

    // set Ai Sound Mode
    eAiSoundMode = 1;//(MI_U32)atoi(optarg);
    if (eAiSoundMode < E_AI_SOUND_MODE_MONO || eAiSoundMode > E_AI_SOUND_MODE_QUEUE)
    {
        printf("Illegal sound mode!!!\n");
        return -1;
    }
    
    bAiDumpPcmData = TRUE;

    bCheckPass = checkParam();
    if (FALSE == bCheckPass)
    {
        printf("Fail to check param.\n");
        display_help();
        return -1;
    }
    else
    {
        initParam();
        //printParam();
    }
}

#if 1
int audio_thread(void)
{
    MI_S32      s32Ret = MI_SUCCESS;
    MI_S32      s32Opt = 0;
    
    MI_U32      u32ChnIdx;
    MI_U32      u32ChnCnt;
    
    AiChnPriv_t* priv = (AiChnPriv_t*)&stAiChnPriv[u32ChnIdx];
    MI_AUDIO_Frame_t stAiChFrame;
    MI_AUDIO_AecFrame_t stAecFrame;
	MI_AI_AedResult_t stAedResult; 

    memset(&stAiChFrame, 0, sizeof(MI_AUDIO_Frame_t));
    memset(&stAecFrame, 0, sizeof(MI_AUDIO_AecFrame_t));
    char *pStream = NULL;
    char buffer[1024*10] = {};
    
    SetAudioMode();
    if ((E_AI_SOUND_MODE_QUEUE == eAiSoundMode) || bAiEnableHwAec)
    {
        u32ChnCnt = 1;
    }
    else
    {
        u32ChnCnt = u32AiChnCnt;
    }

    ES8156_Init();
    ES8156_SelectWordLength(16);
    ES8156_ConfigI2s(EN_I2S_MODE_NORMAL);

    // enable ai
    ExecFunc(initAi(), MI_SUCCESS);

    while (g_Exit)
    {
        #if 1
        int i = 0, len = 0;
        char *pBuffer = buffer;

        s32Ret = MI_AI_GetFrame(priv->AiDevId, priv->AiChn, &stAiChFrame, &stAecFrame, -1);
        pStream = stAiChFrame.apSrcPcmVirAddr[0];
        //printf("audio pcm length : %d \n", stAiChFrame.u32SrcPcmLen);
        if (MI_SUCCESS == s32Ret)
        {
            #if 0
            /**********************change stereo to mome***************************
             * Stereo data format : RRRRLLLLRRRRLLLL
             * four datum as a part 
             * *********************************************************************/
            for (i=0; i<stAiChFrame.u32SrcPcmLen/2; i++)
            {
                *pBuffer++ = *pStream++;
                len++;
                if ((len%4)==0)
                {
                    pStream+=4;
                }
            }
            /************************************************************/
            //printf("Ring PCM push len : %d \n", len);
            RingPCMPush(buffer, len);
            #else
            //memcpy(pBuffer, pStream, stAiChFrame.u32SrcPcmLen);
            if (stAiChFrame.u32SrcPcmLen > 0)
                RingPCMPush(pStream, stAiChFrame.u32SrcPcmLen);
            #endif
            
            MI_AI_ReleaseFrame(priv->AiDevId, priv->AiChn,  &stAiChFrame,  NULL);
        }
        else
        {
            printf("Dev%dChn%d get frame failed!!!error:0x%x\n", priv->AiDevId, priv->AiChn, s32Ret);
        }
        #endif
    }

    // disable ai
    if (bEnableAI)
    {
        ExecFunc(deinitAi(), MI_SUCCESS);
    }
    ES8156_Deinit();

	//ExecFunc(freePrivatePool(), MI_SUCCESS);
    //ExecFunc(MI_SYS_Exit(), MI_SUCCESS);
    printf("------------------ audio thread exit ---------------------\n");
    return 0;
}
#endif
