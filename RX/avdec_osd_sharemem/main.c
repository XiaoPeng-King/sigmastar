#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/prctl.h>

#include "mi_vdec.h"
#include "mi_vdec_datatype.h"
#include "mi_common.h"
#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_sys_datatype.h"
#include "mi_panel_datatype.h"
#include "mi_panel.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
#include "mi_ao.h"
#include "mi_ao_datatype.h"
#include "app_rtp.h"

#include "gpio.h"
#include "uart.h"
#include "watchdog.h"
#include "sstardisp.h"
#include "app_osd.h"
#include "logo.h"
#include "sharemem.h"
#include "app_rx_broadcast.h"
#include "app_igmp.h"
#include "init_net.h"
#include "main.h"


#define STCHECKRESULT(result)\
    if (result != MI_SUCCESS)\
    {\
        printf("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
        return 1;\
    }\
    else\
    {\
        printf("(%s %d)exec function pass\n", __FUNCTION__,__LINE__);\
    }

#define VDEC_CHN_ID     0

#define AUDIO_CHN       0
#define MI_AUDIO_SAMPLE_PER_FRAME 1024
#define DMA_BUF_SIZE_8K     (8000)
#define DMA_BUF_SIZE_16K    (16000)
#define DMA_BUF_SIZE_32K    (32000)
#define DMA_BUF_SIZE_48K    (48000)

#define VDEC_INPUT_WIDTH     1920
#define VDEC_INPUT_HEIGHT    1080

#if ENABLE_HDMI
#define AUDIO_DEV       3
#else
#define AUDIO_DEV       0
#endif

#if ENABLE_HDMI
    #define VDEC_OUTPUT_WIDTH     1920
    #define VDEC_OUTPUT_HEIGHT    1080
#else
    #define VDEC_OUTPUT_WIDTH     1024
    #define VDEC_OUTPUT_HEIGHT    600
#endif

SYSTEM_ATTR_s g_system_attr = {
    .display_state.signal_state = EN_NORMAL,
    .display_state.signal_normal = EN_CLOSE_DISPLAY,
	.multicast = "239.255.42.31",
	.serverip = "192.168.36.1",
	.display_flag = TRUE,
	.multicast_change_flag = FALSE,
	.e2prom = FALSE,
};


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

typedef struct _H264_APPEND_INFO
{
    //audio
    unsigned int fs;
    unsigned int audio_bits;
    unsigned int chns;
    //unsigned int audio_handle;
    //video
    unsigned int width;
    unsigned int height;
    unsigned int src_width;
    unsigned int src_height;
    unsigned char frameRate;
    unsigned char refreshRate;
    unsigned char interlace;
} H264_APPEND_INFO_s;
static int VIDEO_INFO_SIZE = sizeof(H264_APPEND_INFO_s);

static MI_S32 g_s32NeedSize;
static MI_U32 u32DmaBufSize;
static MI_S32 s32SoundLayout;
static MI_S32 s32SampleRate;
static MI_BOOL bExit = FALSE;
MI_VDEC_CodecType_e _eCodecType = E_MI_VDEC_CODEC_TYPE_H264;//E_MI_VDEC_CODEC_TYPE_H265;

int sstar_vdec_init(void)
{
    //init vdec
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_VDEC_OutputPortAttr_t stOutputPortAttr;
    MI_VDEC_CHN stVdecChn = VDEC_CHN_ID;
    MI_VDEC_InitParam_t stVdecInitParam;

    memset(&stVdecInitParam, 0, sizeof(MI_VDEC_InitParam_t));
    stVdecInitParam.bDisableLowLatency = false; //false: 不带B帧, true: 带B帧
    STCHECKRESULT(MI_VDEC_InitDev(&stVdecInitParam));

    memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
    stVdecChnAttr.eCodecType   = _eCodecType;
    stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 5;
    stVdecChnAttr.eVideoMode   = E_MI_VDEC_VIDEO_MODE_FRAME;
    stVdecChnAttr.u32BufSize   = 1 * 1920 * 1088; //1 * 1920 * 1080;
    stVdecChnAttr.u32PicWidth  = inVdecWidth;
    stVdecChnAttr.u32PicHeight = inVdecHeight;
    stVdecChnAttr.eDpbBufMode  = E_MI_VDEC_DPB_MODE_NORMAL;
    stVdecChnAttr.u32Priority  = 0;

    STCHECKRESULT(MI_VDEC_CreateChn(stVdecChn, &stVdecChnAttr));
    STCHECKRESULT(MI_VDEC_StartChn(stVdecChn));

    memset(&stOutputPortAttr, 0, sizeof(MI_VDEC_OutputPortAttr_t));
    stOutputPortAttr.u16Width  = OutVdecWidth;
    stOutputPortAttr.u16Height = OutVdecHeight;
    STCHECKRESULT(MI_VDEC_SetOutputPortAttr(0, &stOutputPortAttr));

    MI_SYS_ChnPort_t  stChnPort;
    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId    = E_MI_MODULE_ID_VDEC;
    stChnPort.u32DevId  = 0;
    stChnPort.u32ChnId  = stVdecChn;
    stChnPort.u32PortId = 0;

    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 5));

    //bind vdec 2 disp
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stSrcChnPort.u32DevId = 0;
    stSrcChnPort.u32ChnId = 0;
    stSrcChnPort.u32PortId = 0;

    stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stDstChnPort.u32DevId = 0;
    stDstChnPort.u32ChnId = 0;
    stDstChnPort.u32PortId = 0;

    STCHECKRESULT(MI_SYS_BindChnPort(&stSrcChnPort, &stDstChnPort, 30, 30));

    if(bRota)
    {
        MI_DISP_RotateConfig_t stRotateConfig;
        memset(&stRotateConfig, 0, sizeof(MI_DISP_RotateConfig_t));
        stRotateConfig.eRotateMode      = E_MI_DISP_ROTATE_90;

        MI_DISP_SetVideoLayerRotateMode(0, &stRotateConfig);
    }

    return 0;
}

int sstar_vdec_deInit(void)
{
    //Unbind vdec 2 disp
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stSrcChnPort.u32DevId = 0;
    stSrcChnPort.u32ChnId = 0;
    stSrcChnPort.u32PortId = 0;

    stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stDstChnPort.u32DevId = 0;
    stDstChnPort.u32ChnId = 0;
    stDstChnPort.u32PortId = 0;

    STCHECKRESULT(MI_SYS_UnBindChnPort(&stSrcChnPort, &stDstChnPort));

    STCHECKRESULT(MI_VDEC_StopChn(0));
    STCHECKRESULT(MI_VDEC_DestroyChn(0));
    STCHECKRESULT(MI_VDEC_DeInitDev());

    return 0;
}

int sstar_ao_deinit(void)
{
    MI_AUDIO_DEV AoDevId = AUDIO_DEV;
    MI_AO_CHN AoChn = AUDIO_CHN;

    /* disable ao channel of */
    MI_AO_DisableChn(AoDevId, AoChn);

    /* disable ao device */
    MI_AO_Disable(AoDevId);

    return MI_SUCCESS;
}

int sstar_ao_init(void)
{
    MI_AUDIO_Attr_t stSetAttr;
    MI_AUDIO_Attr_t stGetAttr;
    MI_AUDIO_DEV AoDevId = AUDIO_DEV;
    MI_AO_CHN AoChn = AUDIO_CHN;

    MI_S32 s32SetVolumeDb;
    MI_S32 s32GetVolumeDb;

    //set Ao Attr struct
    memset(&stSetAttr, 0, sizeof(MI_AUDIO_Attr_t));
    stSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    stSetAttr.u32FrmNum = 6;
    stSetAttr.u32PtNumPerFrm = MI_AUDIO_SAMPLE_PER_FRAME;

    if(s32SoundLayout == 2) {
        stSetAttr.u32ChnCnt = 2;
        stSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_STEREO;  // 立体声
        printf("the sound is stereo !!\n");
    }
    else if(s32SoundLayout == 1) {
        stSetAttr.u32ChnCnt = 1;
        stSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;    // 单声道
        printf("the sound is mono !!\n");
    }

    //stSetAttr.eSamplerate = E_MI_AUDIO_SAMPLE_RATE_48000;   // 采样率
    stSetAttr.eSamplerate = s32SampleRate;
    printf("set ao sound layout [%d], sample rate [%d]\n", s32SoundLayout, s32SampleRate);

    g_s32NeedSize = MI_AUDIO_SAMPLE_PER_FRAME * 2 * (stSetAttr.u32ChnCnt);
    if (E_MI_AUDIO_SAMPLE_RATE_8000 == stSetAttr.eSamplerate) {
        u32DmaBufSize = DMA_BUF_SIZE_8K;;
    }
    else if (E_MI_AUDIO_SAMPLE_RATE_16000 == stSetAttr.eSamplerate) {
        u32DmaBufSize = DMA_BUF_SIZE_16K;
    }
    else if (E_MI_AUDIO_SAMPLE_RATE_32000 == stSetAttr.eSamplerate) {
        u32DmaBufSize = DMA_BUF_SIZE_32K;
    }
    else if (E_MI_AUDIO_SAMPLE_RATE_48000 == stSetAttr.eSamplerate) {
        u32DmaBufSize = DMA_BUF_SIZE_48K;
    }

    if (stSetAttr.eSoundmode == E_MI_AUDIO_SOUND_MODE_STEREO) {
        if (g_s32NeedSize > (u32DmaBufSize / 4)) {
            g_s32NeedSize = u32DmaBufSize / 4;
        }
    }
    else if (stSetAttr.eSoundmode == E_MI_AUDIO_SOUND_MODE_MONO) {
        if (g_s32NeedSize > (u32DmaBufSize / 8)) {
            g_s32NeedSize = u32DmaBufSize / 8;
        }
    }

    /* set ao public attr*/
    MI_AO_SetPubAttr(AoDevId, &stSetAttr);

    /* get ao device*/
    MI_AO_GetPubAttr(AoDevId, &stGetAttr);

    /* enable ao device */
    MI_AO_Enable(AoDevId);

    /* enable ao channel of device*/
    MI_AO_EnableChn(AoDevId, AoChn);

    /* if test AO Volume */
    s32SetVolumeDb = 0;
    MI_AO_SetVolume(AoDevId, s32SetVolumeDb);   // 音量[-60dB ~ 30dB]

    /* get AO volume */
    MI_AO_GetVolume(AoDevId, &s32GetVolumeDb);

    return MI_SUCCESS;
}
#if 0
static void * sstar_audio_thread(void* arg)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_Frame_t stAoSendFrame;
    MI_U8 u8TempBuf[MI_AUDIO_SAMPLE_PER_FRAME * 4];
    WaveFileHeader_t g_stWavHeaderInput;

    char *wave_file = (char *)arg;
    printf("try to open wave file : %s\n", wave_file);

    int g_AoReadFd = open((const char *)wave_file, O_RDONLY, 0666);
    if(g_AoReadFd <= 0) {
        printf("open input file failed!!!\n");
        return NULL;
    }

    s32Ret = read(g_AoReadFd, &g_stWavHeaderInput, sizeof(WaveFileHeader_t));
    if (s32Ret < 0) {
        printf("read wav header failed!!!\n");
        return NULL;
    }
    printf("wave file's audio channel layout = %d\n", g_stWavHeaderInput.wave.wChannels);
    printf("wave file's sample rate = %d\n", g_stWavHeaderInput.wave.dwSamplesPerSec);

    // 拉高公版声音功放IO
    system("echo 12 > /sys/class/gpio/export");
    system("echo out > /sys/class/gpio/gpio12/direction");
    system("echo 1 > /sys/class/gpio/gpio12/value");

    while(!bExit)
    {
        s32Ret = read(g_AoReadFd, &u8TempBuf, g_s32NeedSize);
        if(s32Ret != g_s32NeedSize) {
            lseek(g_AoReadFd, sizeof(WaveFileHeader_t), SEEK_SET);
            s32Ret = read(g_AoReadFd, &u8TempBuf, g_s32NeedSize);
            if (s32Ret < 0) {
                printf("input file does not has enough data!!!\n");
                break;
            }
        }
        // 送数据到AO
        memset(&stAoSendFrame, 0x0, sizeof(MI_AUDIO_Frame_t));
        stAoSendFrame.u32Len = s32Ret;          // pcm 数据长度
        stAoSendFrame.apVirAddr[0] = u8TempBuf; // pcm 数据地址
        stAoSendFrame.apVirAddr[1] = NULL;

        do{
            s32Ret = MI_AO_SendFrame(AUDIO_DEV, AUDIO_CHN, &stAoSendFrame, -1);
        }while(s32Ret == MI_AO_ERR_NOBUF);

        if(s32Ret != MI_SUCCESS)
        {
            printf("[Warning]: MI_AO_SendFrame fail, error is 0x%x: \n", s32Ret);
        }
    }

    if (g_AoReadFd > 0) {
        close(g_AoReadFd);
    }

    return NULL;
}
#endif

/********************************************************************************/
#if 0
//#if (!USE_FFMPEG)

#define MAX_ONE_FRM_SIZE (2 * 1024 * 1024)
#define DROP_SLICE_ID (3)

typedef struct ReOrderSlice_s {
    MI_U8 *pos;
    MI_U32 len;
} ReOrderSlice_t;

typedef struct
{
    int startcodeprefix_len;
    unsigned int len;
    unsigned int max_size;
    char *buf;
    unsigned short lost_packets;
} NALU_t;

ReOrderSlice_t _stVdecSlice[100];
MI_U32 _u32SliceIdx = 0;

MI_BOOL _bFrmMode = FALSE;      //根据输入视频是否带B帧设置
MI_BOOL _bReOrderSlice = FALSE;
MI_BOOL _bSleep = FALSE;
MI_BOOL _bDropFrm = FALSE;
MI_BOOL _bChkStreamEnd = FALSE;

static int FindStartCode2 (unsigned char *Buf)
{
    if((Buf[0] != 0) || (Buf[1] != 0) || (Buf[2] != 1))
        return 0;
    else
        return 1;
}

static int FindStartCode3 (unsigned char *Buf)
{
    if((Buf[0] != 0) || (Buf[1] != 0) || (Buf[2] != 0) || (Buf[3] != 1))
        return 0;
    else
        return 1;
}

NALU_t *AllocNALU(int buffersize)
{
    NALU_t *n;
    if ((n = (NALU_t*)calloc (1, sizeof (NALU_t))) == NULL)
    {
        printf("AllocNALU: calloc n error\n");
        return NULL;
    }
    n->max_size = buffersize;
    //printf("AllocNALU: calloc buf size = %d\n", n->max_size);

    if ((n->buf = (char*)calloc (buffersize, sizeof (char))) == NULL)
    {
        free (n);
        printf("AllocNALU: calloc n->buf error\n");
        return NULL;
    }
    return n;
}

void FreeNALU(NALU_t *n)
{
    if (n)
    {
        if (n->buf)
        {
            free(n->buf);
            n->buf=NULL;
        }
        free (n);
    }
}

int GetAnnexbNALU (NALU_t *nalu, MI_S32 chn, FILE *fp)
{
    int pos = 0;
    int StartCodeFound, rewind;
    unsigned char *Buf;
    int info2 = 0, info3 = 0;

    if ((Buf = (unsigned char*)calloc (nalu->max_size , sizeof(char))) == NULL)
    {
        printf("GetAnnexbNALU: Could not allocate Buf memory\n");
        return -1;
    }

    nalu->startcodeprefix_len=3;
    if (3 != fread (Buf, 1, 3, fp))
    {
        free(Buf);
        return -1;
    }
    info2 = FindStartCode2 (Buf);
    if(info2 != 1)
    {
        if(1 != fread(Buf+3, 1, 1, fp))
        {
            free(Buf);
            return -1;
        }
        info3 = FindStartCode3 (Buf);
        if (info3 != 1)
        {
            free(Buf);
            return -1;
        }
        else
        {
            pos = 4;
            nalu->startcodeprefix_len = 4;
        }
    }
    else
    {
        nalu->startcodeprefix_len = 3;
        pos = 3;
    }
    StartCodeFound = 0;
    info2 = 0;
    info3 = 0;
    while (!StartCodeFound)
    {
        if (feof (fp))
        {
            nalu->len = (pos-1);
            memcpy (nalu->buf, &Buf[0], nalu->len);
            free(Buf);
            if (!_bChkStreamEnd)
            {
                fseek(fp, 0, 0);
            }
            else
            {
                printf("end of file...\n");
            }
            return pos-1;
        }
        Buf[pos++] = fgetc (fp);
        info3 = FindStartCode3(&Buf[pos-4]);
        if(info3 != 1)
            info2 = FindStartCode2(&Buf[pos-3]);
        StartCodeFound = (info2 == 1 || info3 == 1);
    }
    rewind = (info3 == 1) ? -4 : -3;
    if (0 != fseek (fp, rewind, SEEK_CUR))
    {
        free(Buf);
        printf("GetAnnexbNALU: Cannot fseek in the bit stream file\n");
    }
    nalu->len = (pos+rewind);
    memcpy (nalu->buf, &Buf[0], nalu->len);
    free(Buf);
    return (pos+rewind);
}

MI_U64 GetOSTime(void)
{
    MI_U64 u64CurTime = 0;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    u64CurTime = ((unsigned long long)(tv.tv_sec))*1000 + tv.tv_usec/1000;
    return u64CurTime;
}

static void * sstar_video_thread(void* arg)
{
    MI_VDEC_CHN vdecChn = 0;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 s32TimeOutMs = 20;
    MI_U32 u32WrPtr = 0;
    MI_S32 s32NaluType = 0;
    MI_U8 *pu8Buf = NULL;
    MI_U8 *pSlice2FrmBuf = NULL;
    MI_U8 *pSliceReOrder = NULL;
    MI_U64 u64Pts = 0;
    MI_U64 u64FisrtSlicePushTime = 0;
    MI_BOOL bFirstFrm = FALSE;
    MI_BOOL frameType = 0;
    MI_U32 u32SliceCount = 0;
    MI_U32 u32FpBackLen = 0; // if send stream failed, file pointer back length
    FILE *fReadFile = NULL;
    NALU_t *pstNalu;
    MI_VDEC_VideoStream_t stVdecStream;

    pSlice2FrmBuf = (unsigned char *)malloc(MAX_ONE_FRM_SIZE);
    if (!pSlice2FrmBuf) {
        printf("ST_VdecSendStream: malloc frame buf error\n");
        return NULL;
    }

    if (_bReOrderSlice) {
        pSliceReOrder = (unsigned char *)malloc(MAX_ONE_FRM_SIZE);
        if (!pSliceReOrder) {
            printf("ST_VdecSendStream: malloc reorder buf error\n");
            return NULL;
        }
    }

    char *video_file = (char *)arg;

    pstNalu = AllocNALU(MAX_ONE_FRM_SIZE);
    if (!pstNalu) {
        return NULL;
    }

    prctl(PR_SET_NAME, "send_thrd_0");

    fReadFile = fopen(video_file, "rb"); //ES
    if (!fReadFile)
    {
        printf("Open %s failed!\n", video_file);
        return NULL;
    }

    printf("Open %s success, vdec chn:%d\n", video_file, vdecChn);
    pu8Buf = (MI_U8 *)malloc(MAX_ONE_FRM_SIZE);
    if (!pu8Buf) {
        printf("ST_VdecSendStream: malloc pbuf error\n");
        return NULL;
    }

    while (!bExit)
    {
        s32Ret = GetAnnexbNALU(pstNalu, vdecChn, fReadFile);
        if (s32Ret <= 0) {
            printf("GetAnnexbNALU: read nal data error\n");
            continue;
        }

        stVdecStream.pu8Addr = (MI_U8 *)pstNalu->buf;
        if(9 == pstNalu->len
            && 0 == *(pstNalu->buf)
            && 0 == *(pstNalu->buf+1)
            && 0 == *(pstNalu->buf+2)
            && 1 == *(pstNalu->buf+3)
            && 0x68 == *(pstNalu->buf+4)
            && 0 == *(pstNalu->buf+pstNalu->len-1))
        {
            stVdecStream.u32Len = 8;
        }
        else {
            stVdecStream.u32Len = pstNalu->len;
        }
        stVdecStream.u64PTS = u64Pts;
        stVdecStream.bEndOfFrame = 1;
        stVdecStream.bEndOfStream = 0;

        if (_bChkStreamEnd && (feof (fReadFile)))
        {
            stVdecStream.bEndOfStream = 1;
            printf("set end of file flag done\n");
        }

        u32FpBackLen = stVdecStream.u32Len; //back length
        if(0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1]
            && 0x00 == stVdecStream.pu8Addr[2] && 0x01 == stVdecStream.pu8Addr[3]
            && (0x65 == stVdecStream.pu8Addr[4] || 0x61 == stVdecStream.pu8Addr[4]
            || 0x26 == stVdecStream.pu8Addr[4] || 0x02 == stVdecStream.pu8Addr[4]
            || 0x41 == stVdecStream.pu8Addr[4]))
        {
            usleep(20 * 1000);    //帧率控制不超过30
        }

        if (_eCodecType == E_MI_VDEC_CODEC_TYPE_H265) {
            if (0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1] && 0x00 == stVdecStream.pu8Addr[2] && 0x01 == stVdecStream.pu8Addr[3]) {
                bFirstFrm = (stVdecStream.pu8Addr[6] & 0x80);
                s32NaluType = (stVdecStream.pu8Addr[4] & 0x7E) >> 1;
            }

            if (0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1] && 0x01 == stVdecStream.pu8Addr[2]) {
                bFirstFrm = (stVdecStream.pu8Addr[5] & 0x80);
                s32NaluType = (stVdecStream.pu8Addr[3] & 0x7E) >> 1;
            }

            if (s32NaluType <= 31) {
                ///frame type
                frameType = 1;
            } else {
                frameType = 0;
            }
        } else {
            if (0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1] && 0x00 == stVdecStream.pu8Addr[2] && 0x01 == stVdecStream.pu8Addr[3]) {
                bFirstFrm = (stVdecStream.pu8Addr[5] & 0x80);
                s32NaluType = stVdecStream.pu8Addr[4] & 0xF;
            }

            if (0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1] && 0x01 == stVdecStream.pu8Addr[2]) {
                bFirstFrm = (stVdecStream.pu8Addr[4] & 0x80);
                s32NaluType = stVdecStream.pu8Addr[3] & 0xF;
            }
            if (1 <= s32NaluType && s32NaluType <= 5) {
                ///frame type
                frameType = 1;
            } else {
                frameType = 0;
            }
        }

        if (bFirstFrm) {
            u32SliceCount = 0;
        }

        if (u64FisrtSlicePushTime == 0) {
            u64FisrtSlicePushTime = GetOSTime();
        }

        if (_bDropFrm && frameType && (GetOSTime() - u64FisrtSlicePushTime > 3000) && (u32SliceCount == DROP_SLICE_ID)) {
            printf("drop slice, id=%d, 0x%02x, type:%d\n", u32SliceCount, stVdecStream.pu8Addr[4], s32NaluType);
            u32SliceCount++;
            continue;
        }

        if (_bFrmMode) {
            if (u32WrPtr && bFirstFrm) {
                MI_U8 *pTmp = stVdecStream.pu8Addr;
                MI_U32 u32TmpLen = stVdecStream.u32Len;

                stVdecStream.pu8Addr = pSlice2FrmBuf;
                stVdecStream.u32Len = u32WrPtr;

                if (_bReOrderSlice && frameType && (GetOSTime() - u64FisrtSlicePushTime > 3000)) {
                    int WLen = 0;
                    for (int sliceID = 0; sliceID < _u32SliceIdx; ++sliceID) {
                        if (sliceID == 0) {
                            memcpy(pSliceReOrder, _stVdecSlice[0].pos, _stVdecSlice[0].len);
                            WLen += _stVdecSlice[0].len;
                        } else {
                            memcpy(pSliceReOrder + WLen, _stVdecSlice[_u32SliceIdx - sliceID].pos, _stVdecSlice[_u32SliceIdx - sliceID].len);
                            WLen += _stVdecSlice[_u32SliceIdx - sliceID].len;
                        }
                    }

                    stVdecStream.pu8Addr = pSliceReOrder;
                    printf("reorder done...\n");
                }

                if (MI_SUCCESS != (s32Ret = MI_VDEC_SendStream(vdecChn, &stVdecStream, s32TimeOutMs)))
                {
                    printf("chn[%d]: MI_VDEC_SendStream %d fail, 0x%X\n", vdecChn, stVdecStream.u32Len, s32Ret);
                    fseek(fReadFile, - u32FpBackLen, SEEK_CUR);
                    continue;
                } else {
                    stVdecStream.pu8Addr = pTmp;
                    stVdecStream.u32Len = u32TmpLen;
                    u32WrPtr = 0;
                    if (_bReOrderSlice) {
                        _u32SliceIdx = 0;
                    }
                }

                usleep(30 * 1000);
            }

            memcpy(pSlice2FrmBuf + u32WrPtr, stVdecStream.pu8Addr, stVdecStream.u32Len);
            if (_bReOrderSlice) {
                _stVdecSlice[_u32SliceIdx].pos = pSlice2FrmBuf + u32WrPtr;
                _stVdecSlice[_u32SliceIdx].len = stVdecStream.u32Len;
                _u32SliceIdx++;
            }
            u32WrPtr += stVdecStream.u32Len;
        } else {
            if (_bSleep) {
            }

            if (MI_SUCCESS != (s32Ret = MI_VDEC_SendStream(vdecChn, &stVdecStream, s32TimeOutMs)))
            {
                //printf("chn[%d]: MI_VDEC_SendStream %d fail, 0x%X\n", vdecChn, stVdecStream.u32Len, s32Ret);
                fseek(fReadFile, - u32FpBackLen, SEEK_CUR);
                usleep(30 * 1000);
            }
        }

        if (_bChkStreamEnd && (feof (fReadFile)))
        {
            printf("end of stream, wait dec done...\n");
            usleep(5 * 1000 * 1000);
            bExit = TRUE;
        }

        u32SliceCount++;
    }
    free(pu8Buf);
    FreeNALU(pstNalu);
    fclose(fReadFile);
    free(pSlice2FrmBuf);
    printf("end of sstar_video_thread\n");

    return NULL;
}
#endif

void wacPushFrameToMDev(unsigned char *pFrame, unsigned int uiSize)
{
    if (uiSize <= 0)
    {
        return 0;
    }
    
    static FILE *pFile = NULL;
    unsigned char *data = NULL;
    
#if 0 //save 264 video data 
    unsigned int size = uiSize;
	data = (unsigned char *)pFrame + 4;
	if (0x67 == *data) //I frame
	{
		printf("this is I frame \n");
		size = uiSize - sizeof(H264_APPEND_INFO_s);
	}

	if (pFile == NULL)
	{
		pFile = fopen("./test.264", "w+");
	}
	fwrite(pFrame, size, 1, pFile);
	//printf("---size : %d ----\n", size);
	return 0;
#endif
    
	MI_VDEC_VideoStream_t stVdecStream;
    data = (unsigned char *)pFrame + 4;
    if (0x67 == *data)
    {
        stVdecStream.pu8Addr = pFrame;
        stVdecStream.u32Len = uiSize-VIDEO_INFO_SIZE;
        stVdecStream.u64PTS = 0;
        stVdecStream.bEndOfFrame = 1;
        stVdecStream.bEndOfStream = 0;
    }
    else
    {
        stVdecStream.pu8Addr = pFrame;
        stVdecStream.u32Len = uiSize;
        stVdecStream.u64PTS = 0;
        stVdecStream.bEndOfFrame = 1;
        stVdecStream.bEndOfStream = 0;
    }
	
	MI_VDEC_SendStream(0, &stVdecStream, 20);
	return 0;
}

static FILE *fd = 0;
void audio_play(unsigned char *pPCM, unsigned int uiSize)
{
    if (uiSize <= 0)
    {
        printf("pcm data <= 0 \n");
        return 0;
    }
    #if 0
    if (fd == 0)
    {
        fd = fopen("./audio.pcm", "w+");
    }
    fwrite(pPCM, 1, uiSize, fd);
    return 0;
    #endif
    MI_AUDIO_Frame_t stAoSendFrame;
    //送数据到AO
    memset(&stAoSendFrame, 0x0, sizeof(MI_AUDIO_Frame_t));
    stAoSendFrame.u32Len = uiSize;     // pcm 数据长度
    stAoSendFrame.apVirAddr[0] = pPCM; // pcm 数据地址
    stAoSendFrame.apVirAddr[1] = NULL;
    stAoSendFrame.eBitwidth = 0; //0: 16bit 1: 24bit
    stAoSendFrame.eSoundmode = 0; // 0 : mono 1 : stereo
#if 0
    printf("stAoSendFrame.au32PoolId : %d \n", stAoSendFrame.au32PoolId);
    printf("stAoSendFrame.eBitwidth : %d \n", stAoSendFrame.eBitwidth);
    printf("stAoSendFrame.eSoundmode : %d \n", stAoSendFrame.eSoundmode);
    printf("stAoSendFrame.u32Len : %d \n", stAoSendFrame.u32Len);
    printf("stAoSendFrame.u32SrcPcmLen : %d \n", stAoSendFrame.u32SrcPcmLen);
#endif
    MI_AO_SendFrame(AUDIO_DEV, AUDIO_CHN, &stAoSendFrame, -1);
	return 0;
}

static int set_parameter(void)
{
    inVdecWidth  = VDEC_INPUT_WIDTH;
    inVdecHeight = VDEC_INPUT_HEIGHT;//VDEC输入宽高无须用户根据视频宽高设置,最大1080P即可
    OutVdecWidth  = 1920;
    OutVdecHeight = 1080; //VDEC输出宽高与DISP输出宽高设置一致即可
    OutX = 0;
    OutY = 0;
    OutDispWidth  = 1920;
    OutDispHeight = 1080;

    bRota = 0;

    s32SoundLayout = 2;//1; //1 : mono 2 : storeo
    s32SampleRate  = 48000;//44100;

    return 0;
}

static int init_system(void)
{
    init_gpio();
    InitShareMem();
    sstar_vdec_init(); //vdec init   
    sstar_ao_init(); //
    init_eth();
    sleep(1);
}

int main (int argc, char **argv)
{
    pthread_t watchdog_handle;
    pthread_t switch_ip_handle;
    pthread_t uart_watchdog_handle;
    pthread_t rtp_main_handle;
    pthread_t sharemem_main_handle;//
    pthread_t IP_broadcast_recive_handle;
    pthread_t IP_broadcast_report_handle;
    pthread_t IGMP_report_handle;

    MI_DISP_PubAttr_t stDispPubAttr;
    
    set_parameter();
    
#if ENABLE_HDMI
    stDispPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
    stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
#else
    stDispPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
    stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
#endif
    sstar_disp_init(&stDispPubAttr);
    init_system();
    logo_display();

    CreateThread(&watchdog_handle, NULL, watchdog_main, NULL);
    CreateThread(&switch_ip_handle, NULL, switch_ip, NULL);
    CreateThread(&uart_watchdog_handle, NULL, uart_main, NULL);
    CreateThread(&sharemem_main_handle, NULL, sharemem_handle, NULL);
    CreateThread(&rtp_main_handle, NULL, app_rtp_main, NULL);
    CreateThread(&IP_broadcast_recive_handle, NULL, IP_broadcast_recive, NULL);
    CreateThread(&IP_broadcast_report_handle, NULL, IP_broadcast_report, NULL);
    CreateThread(&IGMP_report_handle, NULL, app_igmp_report, NULL);
    
    char string[100] = {};
    osd_disable();
    while (1)
    {
        //printf("g_system_attr.display_state.signal_state: %d \n", g_system_attr.display_state.signal_state);
        switch (g_system_attr.display_state.signal_state)
        {
            case EN_ABNORMAL:
                //printf("g_system_attr.display_state.signal_abnormal: %d \n", g_system_attr.display_state.signal_abnormal);
                switch (g_system_attr.display_state.signal_abnormal)
                {
                    case EN_NO_HDMI:
                        sprintf(string, "ID-%s: is checking TX-%s HDMI signal!", share_mem->sm_eth_setting.strEthIp, share_mem->sm_eth_setting.strEthMulticast);
                        osd_display(20, 20, string);
                        printf(string);
                        break;

                    case EN_NO_TX:
                        sprintf(string, "ID-%s: is searching TX-%s device!", share_mem->sm_eth_setting.strEthIp, share_mem->sm_eth_setting.strEthMulticast);
                        osd_display(20, 20, string);
                        printf(string);
                        break;
                }
                break;

            case EN_NORMAL:
                //printf("g_system_attr.display_state.signal_normal: %d \n", g_system_attr.display_state.signal_normal);
                switch (g_system_attr.display_state.signal_normal)
                {
                    case EN_IP_CONFLICT:
                        sprintf(string, "ID-%s: is conflicting", share_mem->sm_eth_setting.strEthIp);
                        osd_display(20, 20, string);
                        printf(string);
                        break;
                    
                    case EN_INFO_DIAPLAY:
                        sprintf(string, "IP : %s MULTICAST : %s", share_mem->sm_eth_setting.strEthIp, share_mem->sm_eth_setting.strEthMulticast);
                        printf(string);
                        osd_display(20, 20, string);
                        break;
                    
                    case EN_CLOSE_DISPLAY:
                        
                        osd_disable();
                        break;
                }
               
                break;
        }
        //printf("");
        sleep(1);
        //printf("key : %d \n", get_key_value());
    }

    pthread_join(sharemem_main_handle, NULL);
    pthread_join(rtp_main_handle, NULL);
    pthread_join(IP_broadcast_recive_handle, NULL);
    pthread_join(IP_broadcast_report_handle, NULL);
    pthread_join(IGMP_report_handle, NULL);

    sstar_disp_Deinit(&stDispPubAttr);
    sstar_vdec_deInit();

    return 0;
}
