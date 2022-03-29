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
//#include "common.h"
#if 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>

#include "mi_common.h"
#include "watchdog.h"
#include "st_main_venc.h"
#include "mi_sensor.h"
#include "mi_vif.h"
#include "mi_vpe.h"
#include "mi_sensor_datatype.h"
#include "video_ringfifo.h"
#include "app_rtp_tx.h"
#include "sharemem.h"
#include "app_tx_broadcast.h"
#include "app_hot_backup.h"
#include "gpio.h"
#include "uart_watchdog.h"
#include "hdmi_info.h"
#include "version.h"
#include "digit_led.h"

#define ASCII_COLOR_RED                          "\033[1;31m"
#define ASCII_COLOR_WHITE                        "\033[1;37m"
#define ASCII_COLOR_YELLOW                       "\033[1;33m"
#define ASCII_COLOR_BLUE                         "\033[1;36m"
#define ASCII_COLOR_GREEN                        "\033[1;32m"
#define ASCII_COLOR_END                          "\033[0m"

#define venc_info(fmt, args...)     ({do{printf(ASCII_COLOR_WHITE"[APP INFO]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}while(0);})
#define venc_err(fmt, args...)      ({do{printf(ASCII_COLOR_RED  "[APP ERR ]:%s[%d]: ", __FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}while(0);})
#define RGB_BAYER_PIXEL(BitMode, PixelID) (E_MI_SYS_PIXEL_FRAME_RGB_BAYER_BASE+ BitMode*E_MI_SYS_PIXEL_BAYERID_MAX+ PixelID)

#define VENC_CONFIG_PATH "venc_config_264.json"
#define MAX_VENC_NUM    16
#define MAX_ROI_NUM     8
#define MAX_SEI_LEN     16
#define MAX_JSON_STR_LEN    16

unsigned char g_Exit = 1;
char g_HDMI_STATE = -1;

typedef enum {
    EN_NOREMAL = 0,
    EN_NO_SIGNAL, 
    EN_NO_HDMI
}SYSTEM_STATE_e;
SYSTEM_STATE_e g_enSystem_state = EN_NOREMAL;

typedef struct
{
    bool bStart;
    MI_VENC_CHN venc_chn;
    int yuv_width;
    int yuv_height;
    int yuv_format;
    char *yuv_path;
    MI_U32 pic_width;
    MI_U32 pic_height;
    char *es_path;
    MI_U32 bitrate;
    MI_U32 qp;
    MI_U32 gop;
    MI_U32 fps;
    MI_U32 profile;
    MI_VENC_ModType_e eModType;
    MI_VENC_RcMode_e eRcMode;
    int vencFd;

    uint32_t test_flag;
    MI_U8 ins_data[MAX_SEI_LEN];
    MI_U8 ins_len;
    MI_VENC_RoiCfg_t roi[MAX_ROI_NUM];
    int roi_num;
    MI_VENC_ParamH264Entropy_t entropy;
    MI_VENC_ParamRef_t ref;
    MI_VENC_CropCfg_t crop;
    MI_VENC_SuperFrameCfg_t supfrm;
    DECL_VS2(MI_VENC_Param, SliceSplit_t, slisplt)
    DECL_VS2(MI_VENC_Param, IntraPred_t, intra)
    DECL_VS2(MI_VENC_Param, InterPred_t, inter)
    DECL_VS2(MI_VENC_Param, Trans_t, trans)
    DECL_VS2(MI_VENC_Param, Vui_t, vui)
    DECL_VS2(MI_VENC_Param, Dblk_t, dblk)

    pthread_t pt_input;
    pthread_t pt_output;
} VENC_ChnConf_t;

typedef struct
{
    //config reader
    FILE* fd; //config file 
    char buff[128]; //
    uint16_t pos;

    //channel config
    VENC_ChnConf_t conf[2];
} VENC_Ctx_t;
#endif



static MI_BOOL g_bExit = FALSE;
static MI_S32 g_skipFrame = 35;
static MI_U32 g_frameCnt = 0;
//static MI_S32 g_dumpFrame = 60;

//extern unsigned int PutH264DataToBuffer(unsigned char *pFrame,unsigned int size);

void venc_print_help(const char *porgName)
{
    printf("%s [-options] source.yuv\n", porgName);
    printf(" -s <width> <height> .... source resolution.\n");
    printf(" -p <yuv format> ........ 0: yuv420, 1: yuv422.\n");
    printf(" -h ..................... print this help\n");
    exit(0);
}

void venc_parse_options(VENC_Ctx_t *ctx)
{
    ctx->conf[0].yuv_width = h264_append_info.width;//1920; //atoi(argv[++i]);
    ctx->conf[0].yuv_height = h264_append_info.height;//1080; //atoi(argv[++i]);
    printf("ctx->conf[0].yuv_width : %d \n", ctx->conf[0].yuv_width);
    printf("ctx->conf[0].yuv_height : %d \n", ctx->conf[0].yuv_height);


    ctx->conf[0].yuv_format = 0; //atoi(argv[++i]);

    ctx->conf[0].yuv_path = 0;//argv[i];
}

int venc_skip_blank_space(VENC_Ctx_t * ctx)
{
    assert(ctx);
    while(1)
    {
        if(ctx->pos >= sizeof(ctx->buff) || ctx->buff[ctx->pos] == '\n'
                || ctx->buff[ctx->pos] == '\0')
        {
            if(fgets(ctx->buff, sizeof(ctx->buff), ctx->fd) == NULL)
                return -1;
            ctx->pos = 0;
        }
        if(ctx->buff[ctx->pos] != ' ' && ctx->buff[ctx->pos] != '\t')
            return 0;
        ctx->pos++;
    }
}

int venc_find_next_char(VENC_Ctx_t * ctx, char c)
{
    assert(ctx);
    while(1)
    {
        if(ctx->pos >= sizeof(ctx->buff) || ctx->buff[ctx->pos] == '\n')
        {
            if(fgets(ctx->buff, sizeof(ctx->buff), ctx->fd) == NULL)
                return -1;
            ctx->pos = 0;
        }
        if(ctx->buff[ctx->pos] == c)
            return 0;
        ctx->pos++;
    }
}

char venc_get_next_char(VENC_Ctx_t * ctx)
{
    assert(ctx);
    char c = 0;
    ctx->pos++;
    if(venc_skip_blank_space(ctx))
        return -1;
    c = ctx->buff[ctx->pos];
    return c;
}

char venc_get_curr_char(VENC_Ctx_t * ctx)
{
    assert(ctx);
    char c = 0;
    if(venc_skip_blank_space(ctx))
        return -1;
    c = ctx->buff[ctx->pos];
    return c;
}

int venc_get_item_name(VENC_Ctx_t * ctx, char* c, size_t size)
{
    int n = 0;
    assert(ctx && c);
    assert(size > 0);

    venc_skip_blank_space(ctx);
    if(ctx->buff[ctx->pos] != '\"')
        return 1;
    ctx->pos++;
    while(n < size)
    {
        if(ctx->pos >= sizeof(ctx->buff) || ctx->buff[ctx->pos] == '\n')
        {
            if(fgets(ctx->buff, sizeof(ctx->buff), ctx->fd) == NULL)
                return -1;
            ctx->pos = 0;
        }
        if(ctx->buff[ctx->pos] == '\"')
        {
            ctx->pos++;
            c[n] = '\0';
            return 0;
        }
        c[n] = ctx->buff[ctx->pos];
        n++;
        ctx->pos++;
    }
    venc_err("char array size too small.\n");
    return -1;
}

int venc_get_item_value(VENC_Ctx_t * ctx, char* c, size_t size)
{
    int n = 0;
    bool start = false;
    assert(ctx && c);
    assert(size > 0);

    while(n < size)
    {
        if(ctx->pos >= sizeof(ctx->buff) || ctx->buff[ctx->pos] == '\n')
        {
            if(fgets(ctx->buff, sizeof(ctx->buff), ctx->fd) == NULL)
                return -1;
            ctx->pos = 0;
        }
        if(ctx->buff[ctx->pos] == ':')
        {
            ctx->pos++;
            start = true;
            venc_skip_blank_space(ctx);
            continue;
        }
        if(start)
        {
            if(ctx->buff[ctx->pos] != ' ' && ctx->buff[ctx->pos] != ',')
            {
                c[n] = ctx->buff[ctx->pos];
                c[n+1] = '\0';
                n++;
            }
            else
                return 0;
        }
        ctx->pos++;
    }
    venc_err("char array size too small.\n");
    return -1;

}

int venc_check_attr_config(VENC_Ctx_t *ctx, int chn)
{
    char name[MAX_JSON_STR_LEN];
    char val_c[MAX_JSON_STR_LEN];
    assert(ctx);
    assert(chn<2 && chn>=0);

    venc_find_next_char(ctx, '{');
    ctx->pos++;

    while(venc_get_item_name(ctx, name, MAX_JSON_STR_LEN) == 0)
    {
        venc_get_item_value(ctx, val_c, MAX_JSON_STR_LEN);
        if(!strcmp(name, "chn"))
            ctx->conf[chn].venc_chn = atoi(val_c);
        else if(!strcmp(name, "w"))
            ctx->conf[chn].pic_width = atoi(val_c);
        else if(!strcmp(name, "h"))
            ctx->conf[chn].pic_height = atoi(val_c);
        else if(!strcmp(name, "codec"))
        {
            if(!strcmp(val_c, "\"h264\""))
                ctx->conf[chn].eModType = E_MI_VENC_MODTYPE_H264E;
            else if(!strcmp(val_c, "\"h265\""))
                ctx->conf[chn].eModType = E_MI_VENC_MODTYPE_H265E;
            else
                ctx->conf[chn].eModType = E_MI_VENC_MODTYPE_JPEGE;
        }
        else if(!strcmp(name, "gop"))
            ctx->conf[chn].gop = atoi(val_c);
        else if(!strcmp(name, "rc"))
        {
            if(!strcmp(val_c, "\"cbr\""))
                if(ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H264E)
                    ctx->conf[chn].eRcMode = E_MI_VENC_RC_MODE_H264CBR;
                else if(ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H265E)
                    ctx->conf[chn].eRcMode = E_MI_VENC_RC_MODE_H265CBR;
                else
                    ctx->conf[chn].eRcMode = E_MI_VENC_RC_MODE_MJPEGCBR;
            else if(!strcmp(val_c, "\"vbr\""))
                if(ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H264E)
                    ctx->conf[chn].eRcMode = E_MI_VENC_RC_MODE_H264VBR;
                else if(ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H265E)
                    ctx->conf[chn].eRcMode = E_MI_VENC_RC_MODE_H265VBR;
                else
                    venc_err("jpeg is not support vbr.");
            else
                if(ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H264E)
                    ctx->conf[chn].eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
                else if(ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H265E)
                    ctx->conf[chn].eRcMode = E_MI_VENC_RC_MODE_H265FIXQP;
                else
                    ctx->conf[chn].eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
        }
        else if(!strcmp(name, "bps"))
            ctx->conf[chn].bitrate = atoi(val_c);
        else if(!strcmp(name, "qp"))
            ctx->conf[chn].qp = atoi(val_c);
        else if(!strcmp(name, "fps"))
        {
            ctx->conf[chn].fps = atoi(val_c);
        }
        else if(!strcmp(name, "prof"))
        {
            ctx->conf[chn].profile = atoi(val_c);
        }

        if(venc_get_curr_char(ctx) != ',')
        {
            break;
        }
        ctx->pos++; // skip ','
    }

    venc_find_next_char(ctx, '}'); //go to chn config end
    ctx->pos++; // skip '}'

    return -1;
}

int venc_check_ins_config(VENC_Ctx_t *ctx, int chn)
{
    char name[MAX_JSON_STR_LEN];
    char val_c[MAX_JSON_STR_LEN];
    int set = 0;

    if(venc_get_next_char(ctx) != '{')
        return -1;

    ctx->pos++;

    while(venc_get_item_name(ctx, name, MAX_JSON_STR_LEN) == 0)
    {
        venc_get_item_value(ctx, val_c, MAX_JSON_STR_LEN);
        if(!strcmp(name, "set"))
        {
            set = atoi(val_c);
            venc_info("ins:%d\n", set);
            if(set)
                ctx->conf[chn].test_flag |= VENC_TEST_INSERT;
            else
                ctx->conf[chn].test_flag &= ~VENC_TEST_INSERT;
        }
        else if(!strcmp(name, "data"))
        {
            memset(ctx->conf[chn].ins_data, 0, sizeof(ctx->conf[chn].ins_data));
            memcpy(ctx->conf[chn].ins_data, val_c, strlen(val_c));
            ctx->conf[chn].ins_len = strlen(val_c);
        }

        if(venc_get_curr_char(ctx) != ',')
        {
            break;
        }
        ctx->pos++; // skip ','
    }

    venc_find_next_char(ctx, '}'); //go to chn config end
    ctx->pos++; // skip '}'
    return 0;
}

int venc_check_entropy_config(VENC_Ctx_t *ctx, int chn)
{
    char name[MAX_JSON_STR_LEN];
    char val_c[MAX_JSON_STR_LEN];
    int set = 0;

    if(venc_get_next_char(ctx) != '{')
        return -1;

    ctx->pos++;

    while(venc_get_item_name(ctx, name, MAX_JSON_STR_LEN) == 0)
    {
        venc_get_item_value(ctx, val_c, MAX_JSON_STR_LEN);
        if(!strcmp(name, "set"))
        {
            set = atoi(val_c);
            venc_info("entropy:%d\n", set);
            if(set)
                ctx->conf[chn].test_flag |= VENC_TEST_ENTROPY;
            else
                ctx->conf[chn].test_flag &= ~VENC_TEST_ENTROPY;
        }
        else if(!strcmp(name, "mode_i"))           ctx->conf[chn].entropy.u32EntropyEncModeI = atoi(val_c);
        else if(!strcmp(name, "mode_p"))           ctx->conf[chn].entropy.u32EntropyEncModeP = atoi(val_c);

        if(venc_get_curr_char(ctx) != ',')
        {
            break;
        }
        ctx->pos++; // skip ','
    }

    venc_find_next_char(ctx, '}'); //go to chn config end
    ctx->pos++; // skip '}'
    return 0;
}

int venc_check_ref_config(VENC_Ctx_t *ctx, int chn)
{
    char name[MAX_JSON_STR_LEN];
    char val_c[MAX_JSON_STR_LEN];
    int set = 0;

    if(venc_get_next_char(ctx) != '{')
        return -1;

    ctx->pos++;

    while(venc_get_item_name(ctx, name, MAX_JSON_STR_LEN) == 0)
    {
        venc_get_item_value(ctx, val_c, MAX_JSON_STR_LEN);
        if(!strcmp(name, "set"))
        {
            set = atoi(val_c);
            venc_info("ref:%d\n", set);
            if(set)
                ctx->conf[chn].test_flag |= VENC_TEST_REF;
            else
                ctx->conf[chn].test_flag &= ~VENC_TEST_REF;
        }
        else if(!strcmp(name, "base"))      ctx->conf[chn].ref.u32Base = atoi(val_c);
        else if(!strcmp(name, "enhance"))   ctx->conf[chn].ref.u32Enhance = atoi(val_c);
        else if(!strcmp(name, "pred"))      ctx->conf[chn].ref.bEnablePred = atoi(val_c);

        if(venc_get_curr_char(ctx) != ',')
        {
            break;
        }
        ctx->pos++; // skip ','
    }

    venc_find_next_char(ctx, '}'); //go to chn config end
    ctx->pos++; // skip '}'
    return 0;
}

int venc_check_crop_config(VENC_Ctx_t *ctx, int chn)
{
    char name[MAX_JSON_STR_LEN];
    char val_c[MAX_JSON_STR_LEN];
    int set = 0;

    if(venc_get_next_char(ctx) != '{')
        return -1;

    ctx->pos++;

    while(venc_get_item_name(ctx, name, MAX_JSON_STR_LEN) == 0)
    {
        venc_get_item_value(ctx, val_c, MAX_JSON_STR_LEN);
        if(!strcmp(name, "set"))
        {
            set = atoi(val_c);
            venc_info("crop:%d\n", set);
            if(set)
                ctx->conf[chn].test_flag |= VENC_TEST_CROP;
            else
                ctx->conf[chn].test_flag &= ~VENC_TEST_CROP;
        }
        else if(!strcmp(name, "enable"))      ctx->conf[chn].crop.bEnable = atoi(val_c);
        else if(!strcmp(name, "left"))   ctx->conf[chn].crop.stRect.u32Left = atoi(val_c);
        else if(!strcmp(name, "top"))      ctx->conf[chn].crop.stRect.u32Top = atoi(val_c);
        else if(!strcmp(name, "w"))      ctx->conf[chn].crop.stRect.u32Width = atoi(val_c);
        else if(!strcmp(name, "h"))      ctx->conf[chn].crop.stRect.u32Height = atoi(val_c);

        if(venc_get_curr_char(ctx) != ',')
        {
            break;
        }
        ctx->pos++; // skip ','
    }

    venc_find_next_char(ctx, '}'); //go to chn config end
    ctx->pos++; // skip '}'
    return 0;
}

int venc_check_slisplt_config(VENC_Ctx_t *ctx, int chn)
{
    char name[MAX_JSON_STR_LEN];
    char val_c[MAX_JSON_STR_LEN];
    int set = 0;

    if(venc_get_next_char(ctx) != '{')
        return -1;

    ctx->pos++;

    while(venc_get_item_name(ctx, name, 16) == 0)
    {
        venc_get_item_value(ctx, val_c, 16);
        if(!strcmp(name, "set"))
        {
            set = atoi(val_c);
            venc_info("slisplt:%d\n", set);
            if(set)
                ctx->conf[chn].test_flag |= VENC_TEST_SLISPLT;
            else
                ctx->conf[chn].test_flag &= ~VENC_TEST_SLISPLT;
        }
        else if(!strcmp(name, "enable"))
        {
            if(ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H264E)
                ctx->conf[chn].slisplt_h264.bSplitEnable = atoi(val_c);
            else
                ctx->conf[chn].slisplt_h265.bSplitEnable = atoi(val_c);
        }
        else if(!strcmp(name, "row"))
        {
            if(ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H264E)
                ctx->conf[chn].slisplt_h264.u32SliceRowCount = atoi(val_c);
            else
                ctx->conf[chn].slisplt_h265.u32SliceRowCount = atoi(val_c);
        }

        if(venc_get_curr_char(ctx) != ',')
        {
            break;
        }
        ctx->pos++; // skip ','
    }

    venc_find_next_char(ctx, '}'); //go to chn config end
    ctx->pos++; // skip '}'
    return 0;
}

int venc_check_intra_config(VENC_Ctx_t *ctx, int chn)
{
    char name[MAX_JSON_STR_LEN];
    char val_c[MAX_JSON_STR_LEN];
    int set = 0;
    bool is_h264 = 0;

    if(venc_get_next_char(ctx) != '{')
        return -1;

    ctx->pos++;

    while(venc_get_item_name(ctx, name, MAX_JSON_STR_LEN) == 0)
    {
        venc_get_item_value(ctx, val_c, MAX_JSON_STR_LEN);
        if(!strcmp(name, "set"))
        {
            set = atoi(val_c);
        }
        else if(!strcmp(name, "codec"))
        {
            if(!strcmp(val_c, "\"h264\""))      is_h264 = true;
            else if(!strcmp(val_c, "\"h265\"")) is_h264 = false;
            else venc_err("intra format error.");

            if((is_h264 && ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H264E) ||
                (!is_h264 && ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H265E))
            {
                venc_info("intra:%d\n", set);
                if(set)
                    ctx->conf[chn].test_flag |= VENC_TEST_INTRA;
                else
                    ctx->conf[chn].test_flag &= ~VENC_TEST_INTRA;
            }
        }
        else if(!strcmp(name, "16x16_en"))
        {
            if(is_h264)
                ctx->conf[chn].intra_h264.bIntra16x16PredEn = atoi(val_c);
            else
                ctx->conf[chn].intra_h265.bIntra16x16PredEn = atoi(val_c);
        }
        else if(!strcmp(name, "nxn_en"))
            ctx->conf[chn].intra_h264.bIntraNxNPredEn = atoi(val_c);
        else if(!strcmp(name, "cipf"))
        {
            if(is_h264)
                ctx->conf[chn].intra_h264.bConstrainedIntraPredFlag = atoi(val_c);
            else
                ctx->conf[chn].intra_h265.bConstrainedIntraPredFlag = atoi(val_c);
        }
        else if(!strcmp(name, "ipcm"))
            ctx->conf[chn].intra_h264.bIpcmEn = atoi(val_c);
        else if(!strcmp(name, "16x16p"))
        {
            if(is_h264)
                ctx->conf[chn].intra_h264.u32Intra16x16Penalty = atoi(val_c);
            else
                ctx->conf[chn].intra_h265.u32Intra16x16Penalty = atoi(val_c);
        }
        else if(!strcmp(name, "4x4p"))
            ctx->conf[chn].intra_h264.u32Intra4x4Penalty = atoi(val_c);
        else if(!strcmp(name, "ipp"))
            ctx->conf[chn].intra_h264.bIntraPlanarPenalty = atoi(val_c);
        else if(!strcmp(name, "32x32_en"))
            ctx->conf[chn].intra_h265.bIntra32x32PredEn = atoi(val_c);
        else if(!strcmp(name, "8x8_en"))
            ctx->conf[chn].intra_h265.bIntra8x8PredEn = atoi(val_c);
        else if(!strcmp(name, "32x32p"))
            ctx->conf[chn].intra_h265.u32Intra32x32Penalty = atoi(val_c);
        else if(!strcmp(name, "8x8p"))
            ctx->conf[chn].intra_h265.u32Intra8x8Penalty = atoi(val_c);

        if(venc_get_curr_char(ctx) != ',')
        {
            break;
        }
        ctx->pos++; // skip ','
    }

    venc_find_next_char(ctx, '}'); //go to chn config end
    ctx->pos++; // skip '}'
    return 0;
}

int venc_check_inter_config(VENC_Ctx_t *ctx, int chn)
{
    char name[MAX_JSON_STR_LEN];
    char val_c[MAX_JSON_STR_LEN];
    int set = 0;
    bool is_h264 = 0;

    if(venc_get_next_char(ctx) != '{')
        return -1;

    ctx->pos++;

    while(venc_get_item_name(ctx, name, MAX_JSON_STR_LEN) == 0)
    {
        venc_get_item_value(ctx, val_c, MAX_JSON_STR_LEN);
        if(!strcmp(name, "set"))
        {
            set = atoi(val_c);
        }
        else if(!strcmp(name, "codec"))
        {
            if(!strcmp(val_c, "\"h264\""))      is_h264 = true;
            else if(!strcmp(val_c, "\"h265\"")) is_h264 = false;
            else venc_err("inter format error.");

            if((is_h264 && ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H264E) ||
                (!is_h264 && ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H265E))
            {
                venc_info("inter:%d\n", set);
                if(set)
                    ctx->conf[chn].test_flag |= VENC_TEST_INTER;
                else
                    ctx->conf[chn].test_flag &= ~VENC_TEST_INTER;
            }
        }
        else if(!strcmp(name, "h-size"))
        {
            if(is_h264)
                ctx->conf[chn].inter_h264.u32HWSize = atoi(val_c);
            else
                ctx->conf[chn].inter_h265.u32HWSize = atoi(val_c);
        }
        else if(!strcmp(name, "v-size"))
        {
            if(is_h264)
                ctx->conf[chn].inter_h264.u32VWSize = atoi(val_c);
            else
                ctx->conf[chn].inter_h265.u32VWSize = atoi(val_c);
        }
        else if(!strcmp(name, "16x16_en"))
        {
            if(is_h264)
                ctx->conf[chn].inter_h264.bInter16x16PredEn = atoi(val_c);
            else
                ctx->conf[chn].inter_h265.bInter16x16PredEn = atoi(val_c);
        }
        else if(!strcmp(name, "16x8_en"))
        {
            if(is_h264)
                ctx->conf[chn].inter_h264.bInter16x8PredEn = atoi(val_c);
            else
                ctx->conf[chn].inter_h265.bInter16x8PredEn = atoi(val_c);
        }
        else if(!strcmp(name, "8x16_en"))
        {
            if(is_h264)
                ctx->conf[chn].inter_h264.bInter8x16PredEn = atoi(val_c);
            else
                ctx->conf[chn].inter_h265.bInter8x16PredEn = atoi(val_c);
        }
        else if(!strcmp(name, "8x8_en"))
        {
            if(is_h264)
                ctx->conf[chn].inter_h264.bInter8x8PredEn = atoi(val_c);
            else
                ctx->conf[chn].inter_h265.bInter8x8PredEn = atoi(val_c);
        }
        else if(!strcmp(name, "8x4_en"))
        {
            if(is_h264)
                ctx->conf[chn].inter_h264.bInter8x4PredEn = atoi(val_c);
            else
                ctx->conf[chn].inter_h265.bInter8x4PredEn = atoi(val_c);
        }
        else if(!strcmp(name, "4x8_en"))
        {
            if(is_h264)
                ctx->conf[chn].inter_h264.bInter4x8PredEn = atoi(val_c);
            else
                ctx->conf[chn].inter_h265.bInter4x8PredEn = atoi(val_c);
        }
        else if(!strcmp(name, "4x4_en"))
        {
            if(is_h264)
                ctx->conf[chn].inter_h264.bInter4x4PredEn = atoi(val_c);
            else
                ctx->conf[chn].inter_h265.bInter4x4PredEn = atoi(val_c);
        }
        else if(!strcmp(name, "ext_en"))
        {
            if(is_h264)
                ctx->conf[chn].inter_h264.bExtedgeEn = atoi(val_c);
            else
                ctx->conf[chn].inter_h265.bExtedgeEn = atoi(val_c);
        }
        else if(!strcmp(name, "32x32p"))
            ctx->conf[chn].inter_h265.u32Inter32x32Penalty = atoi(val_c);
        else if(!strcmp(name, "16x16p"))
            ctx->conf[chn].inter_h265.u32Inter16x16Penalty = atoi(val_c);
        else if(!strcmp(name, "8x8p"))
            ctx->conf[chn].inter_h265.u32Inter8x8Penalty = atoi(val_c);

        if(venc_get_curr_char(ctx) != ',')
        {
            break;
        }
        ctx->pos++; // skip ','
    }

    venc_find_next_char(ctx, '}'); //go to chn config end
    ctx->pos++; // skip '}'
    return 0;
}

int venc_check_roi_config(VENC_Ctx_t *ctx, int chn, int n)
{
    char name[MAX_JSON_STR_LEN];
    char val_c[MAX_JSON_STR_LEN];
    int set = 0;

    if(n >= MAX_ROI_NUM)
        return -1;

    if(venc_get_next_char(ctx) != '{')
        return -1;

    ctx->pos++;

    while(venc_get_item_name(ctx, name, MAX_JSON_STR_LEN) == 0)
    {
        venc_get_item_value(ctx, val_c, MAX_JSON_STR_LEN);
        if(!strcmp(name, "set"))
        {
            set = atoi(val_c);
            venc_info("roi:%d\n", set);
            if(set)
                ctx->conf[chn].test_flag |= VENC_TEST_ROI;
            else
                ctx->conf[chn].test_flag &= ~VENC_TEST_ROI;
        }
        else if(!strcmp(name, "index"))     ctx->conf[chn].roi[n].u32Index = atoi(val_c);
        else if(!strcmp(name, "enable"))    ctx->conf[chn].roi[n].bEnable = atoi(val_c);
        else if(!strcmp(name, "abs"))       ctx->conf[chn].roi[n].bAbsQp = atoi(val_c);
        else if(!strcmp(name, "qp"))        ctx->conf[chn].roi[n].s32Qp = atoi(val_c);
        else if(!strcmp(name, "left"))      ctx->conf[chn].roi[n].stRect.u32Left = atoi(val_c);
        else if(!strcmp(name, "top"))       ctx->conf[chn].roi[n].stRect.u32Top = atoi(val_c);
        else if(!strcmp(name, "w"))       ctx->conf[chn].roi[n].stRect.u32Width = atoi(val_c);
        else if(!strcmp(name, "h"))       ctx->conf[chn].roi[n].stRect.u32Height = atoi(val_c);

        if(venc_get_curr_char(ctx) != ',')
        {
            break;
        }
        ctx->pos++; // skip ','
    }

    venc_find_next_char(ctx, '}'); //go to chn config end
    ctx->pos++; // skip '}'
    return 0;
}

int venc_check_trans_config(VENC_Ctx_t *ctx, int chn)
{
    char name[MAX_JSON_STR_LEN];
    char val_c[MAX_JSON_STR_LEN];
    int set = 0;

    if(venc_get_next_char(ctx) != '{')
        return -1;

    ctx->pos++;

    while(venc_get_item_name(ctx, name, 16) == 0)
    {
        venc_get_item_value(ctx, val_c, 16);
        if(!strcmp(name, "set"))
        {
            set = atoi(val_c);
            venc_info("trans:%d\n", set);
            if(set)
                ctx->conf[chn].test_flag |= VENC_TEST_TRANS;
            else
                ctx->conf[chn].test_flag &= ~VENC_TEST_TRANS;
        }
        else if(!strcmp(name, "intra-m"))
        {
            if(ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H264E)
                ctx->conf[chn].trans_h264.u32IntraTransMode = atoi(val_c);
            else
                ctx->conf[chn].trans_h265.u32IntraTransMode = atoi(val_c);
        }
        else if(!strcmp(name, "inter-m"))
        {
            if(ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H264E)
                ctx->conf[chn].trans_h264.u32InterTransMode = atoi(val_c);
            else
                ctx->conf[chn].trans_h265.u32InterTransMode = atoi(val_c);
        }
        else if(!strcmp(name, "cqio"))
        {
            if(ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H264E)
                ctx->conf[chn].trans_h264.s32ChromaQpIndexOffset = atoi(val_c);
            else
                ctx->conf[chn].trans_h265.s32ChromaQpIndexOffset = atoi(val_c);
        }

        if(venc_get_curr_char(ctx) != ',')
        {
            break;
        }
        ctx->pos++; // skip ','
    }

    venc_find_next_char(ctx, '}'); //go to chn config end
    ctx->pos++; // skip '}'
    return 0;
}

int venc_check_vui_config(VENC_Ctx_t *ctx, int chn)
{
    char name[MAX_JSON_STR_LEN];
    char val_c[MAX_JSON_STR_LEN];
    int set = 0;
    bool is_h264 = false;

    if(venc_get_next_char(ctx) != '{')
        return -1;

    ctx->pos++;

    while(venc_get_item_name(ctx, name, MAX_JSON_STR_LEN) == 0)
    {
        venc_get_item_value(ctx, val_c, MAX_JSON_STR_LEN);
        if(!strcmp(name, "set"))
        {
            set = atoi(val_c);
        }
        else if(!strcmp(name, "codec"))
        {
            if(!strcmp(val_c, "\"h264\""))      is_h264 = true;
            else if(!strcmp(val_c, "\"h265\"")) is_h264 = false;
            else venc_err("vui format error.");

            if((is_h264 && ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H264E) ||
                (!is_h264 && ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H265E))
            {
                venc_info("vui:%d\n", set);
                if(set)
                    ctx->conf[chn].test_flag |= VENC_TEST_VUI;
                else
                    ctx->conf[chn].test_flag &= ~VENC_TEST_VUI;
            }
        }
        else if(!strcmp(name, "asp-ratio-flg"))
        {
            if(is_h264)
                ctx->conf[chn].vui_h264.stVuiAspectRatio.u8AspectRatioInfoPresentFlag = atoi(val_c);
            else
                ctx->conf[chn].vui_h265.stVuiAspectRatio.u8AspectRatioInfoPresentFlag = atoi(val_c);
        }
        else if(!strcmp(name, "asp-ratio-idc"))
        {
            if(is_h264)
                ctx->conf[chn].vui_h264.stVuiAspectRatio.u8AspectRatioInfoPresentFlag = atoi(val_c);
            else
                ctx->conf[chn].vui_h265.stVuiAspectRatio.u8AspectRatioInfoPresentFlag = atoi(val_c);
        }
        else if(!strcmp(name, "ovrscn-if-flg"))
        {
            if(is_h264)
                ctx->conf[chn].vui_h264.stVuiAspectRatio.u8OverscanInfoPresentFlag = atoi(val_c);
            else
                ctx->conf[chn].vui_h265.stVuiAspectRatio.u8OverscanInfoPresentFlag = atoi(val_c);
        }
        else if(!strcmp(name, "ovrscn-ap-flg"))
        {
            if(is_h264)
                ctx->conf[chn].vui_h264.stVuiAspectRatio.u8OverscanAppropriateFlag = atoi(val_c);
            else
                ctx->conf[chn].vui_h265.stVuiAspectRatio.u8OverscanAppropriateFlag = atoi(val_c);
        }
        else if(!strcmp(name, "sar-w"))
        {
            if(is_h264)
                ctx->conf[chn].vui_h264.stVuiAspectRatio.u16SarWidth = atoi(val_c);
            else
                ctx->conf[chn].vui_h265.stVuiAspectRatio.u16SarWidth = atoi(val_c);
        }
        else if(!strcmp(name, "sar-h"))
        {
            if(is_h264)
                ctx->conf[chn].vui_h264.stVuiAspectRatio.u16SarHeight = atoi(val_c);
            else
                ctx->conf[chn].vui_h265.stVuiAspectRatio.u16SarHeight = atoi(val_c);
        }
        else if(!strcmp(name, "time-info-flg"))
        {
            if(is_h264)
                ctx->conf[chn].vui_h264.stVuiTimeInfo.u8TimingInfoPresentFlag = atoi(val_c);
            else
                ctx->conf[chn].vui_h265.stVuiTimeInfo.u8TimingInfoPresentFlag = atoi(val_c);
        }
        else if(!strcmp(name, "fix-frm-flg"))
            ctx->conf[chn].vui_h264.stVuiTimeInfo.u8FixedFrameRateFlag = atoi(val_c);
        else if(!strcmp(name, "num-uts-in-tk"))
        {
            if(is_h264)
                ctx->conf[chn].vui_h264.stVuiTimeInfo.u32NumUnitsInTick = atoi(val_c);
            else
                ctx->conf[chn].vui_h265.stVuiTimeInfo.u32NumUnitsInTick = atoi(val_c);
        }
        else if(!strcmp(name, "time-scl"))
        {
            if(is_h264)
                ctx->conf[chn].vui_h264.stVuiTimeInfo.u32TimeScale = atoi(val_c);
            else
                ctx->conf[chn].vui_h265.stVuiTimeInfo.u32TimeScale = atoi(val_c);
        }
        else if(!strcmp(name, "v-sig-flg"))
        {
            if(is_h264)
                ctx->conf[chn].vui_h264.stVuiVideoSignal.u8VideoSignalTypePresentFlag = atoi(val_c);
            else
                ctx->conf[chn].vui_h265.stVuiVideoSignal.u8VideoSignalTypePresentFlag = atoi(val_c);
        }
        else if(!strcmp(name, "v-formt"))
        {
            if(is_h264)
                ctx->conf[chn].vui_h264.stVuiVideoSignal.u8VideoFormat = atoi(val_c);
            else
                ctx->conf[chn].vui_h265.stVuiVideoSignal.u8VideoFormat = atoi(val_c);
        }
        else if(!strcmp(name, "v-ful-rng-flg"))
        {
            if(is_h264)
                ctx->conf[chn].vui_h264.stVuiVideoSignal.u8VideoFullRangeFlag = atoi(val_c);
            else
                ctx->conf[chn].vui_h265.stVuiVideoSignal.u8VideoFullRangeFlag = atoi(val_c);
        }
        else if(!strcmp(name, "clor-desp-flg"))
        {
            if(is_h264)
                ctx->conf[chn].vui_h264.stVuiVideoSignal.u8ColourDescriptionPresentFlag = atoi(val_c);
            else
                ctx->conf[chn].vui_h265.stVuiVideoSignal.u8ColourDescriptionPresentFlag = atoi(val_c);
        }
        else if(!strcmp(name, "clor-prim"))
        {
            if(is_h264)
                ctx->conf[chn].vui_h264.stVuiVideoSignal.u8ColourPrimaries = atoi(val_c);
            else
                ctx->conf[chn].vui_h265.stVuiVideoSignal.u8ColourPrimaries = atoi(val_c);
        }
        else if(!strcmp(name, "tra-chartic"))
        {
            if(is_h264)
                ctx->conf[chn].vui_h264.stVuiVideoSignal.u8TransferCharacteristics = atoi(val_c);
            else
                ctx->conf[chn].vui_h265.stVuiVideoSignal.u8TransferCharacteristics = atoi(val_c);
        }
        else if(!strcmp(name, "mtx-coff"))
        {
            if(is_h264)
                ctx->conf[chn].vui_h264.stVuiVideoSignal.u8MatrixCoefficients = atoi(val_c);
            else
                ctx->conf[chn].vui_h265.stVuiVideoSignal.u8MatrixCoefficients = atoi(val_c);
        }

        if(venc_get_curr_char(ctx) != ',')
        {
            break;
        }
        ctx->pos++; // skip ','
    }

    venc_find_next_char(ctx, '}'); //go to chn config end
    ctx->pos++; // skip '}'
    return 0;
}

int venc_check_dblk_config(VENC_Ctx_t *ctx, int chn)
{
    char name[MAX_JSON_STR_LEN];
    char val_c[MAX_JSON_STR_LEN];
    int set = 0;
    bool is_h264 = 0;

    if(venc_get_next_char(ctx) != '{')
        return -1;

    ctx->pos++;

    while(venc_get_item_name(ctx, name, MAX_JSON_STR_LEN) == 0)
    {
        venc_get_item_value(ctx, val_c, MAX_JSON_STR_LEN);
        if(!strcmp(name, "set"))
        {
            set = atoi(val_c);
        }
        else if(!strcmp(name, "codec"))
        {
            if(!strcmp(val_c, "\"h264\""))      is_h264 = true;
            else if(!strcmp(val_c, "\"h265\"")) is_h264 = false;
            else venc_err("dblk format error.");

            if((is_h264 && ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H264E) ||
                (!is_h264 && ctx->conf[chn].eModType == E_MI_VENC_MODTYPE_H265E))
            {
                venc_info("dblk:%d\n", set);
                if(set)
                    ctx->conf[chn].test_flag |= VENC_TEST_DBLK;
                else
                    ctx->conf[chn].test_flag &= ~VENC_TEST_DBLK;
            }
        }
        else if(!strcmp(name, "dis-dblk-flt"))
        {
            if(is_h264)
                ctx->conf[chn].dblk_h264.disable_deblocking_filter_idc = atoi(val_c);
            else
                ctx->conf[chn].dblk_h265.disable_deblocking_filter_idc = atoi(val_c);
        }
        else if(!strcmp(name, "sli-beta-off"))
        {
            if(is_h264)
                ctx->conf[chn].dblk_h264.slice_beta_offset_div2 = atoi(val_c);
            else
                ctx->conf[chn].dblk_h265.slice_beta_offset_div2 = atoi(val_c);
        }
        else if(!strcmp(name, "sli-apha-coff"))
            ctx->conf[chn].dblk_h264.slice_alpha_c0_offset_div2 = atoi(val_c);
        else if(!strcmp(name, "sli-tc-off"))
            ctx->conf[chn].dblk_h265.slice_tc_offset_div2 = atoi(val_c);

        if(venc_get_curr_char(ctx) != ',')
        {
            break;
        }
        ctx->pos++; // skip ','
    }

    venc_find_next_char(ctx, '}'); //go to chn config end
    ctx->pos++; // skip '}'
    return 0;
}

int venc_check_config_chn(VENC_Ctx_t *ctx)
{
    char name[MAX_JSON_STR_LEN];
    char val_c[MAX_JSON_STR_LEN];
    int chn = 0;

    if(venc_get_next_char(ctx) != '{')
        return -1;

    ctx->pos++;

    while(venc_get_item_name(ctx, name, MAX_JSON_STR_LEN) == 0)
    {
        if(!strcmp(name, "id"))
        {
            venc_get_item_value(ctx, val_c, MAX_JSON_STR_LEN);
            chn = atoi(val_c);
            venc_info("chn:%d\n", chn);
        }
        else if(!strcmp(name, "attr"))      venc_check_attr_config(ctx, chn);
        else if(!strcmp(name, "ins"))       venc_check_ins_config(ctx, chn);
        else if(!strcmp(name, "roi"))
        {
            venc_find_next_char(ctx, ':');
            if(venc_get_next_char(ctx) == '[')
            {
                ctx->conf[chn].roi_num = 0;
                do{
                    ctx->pos++;
                    venc_check_roi_config(ctx, chn, ctx->conf[chn].roi_num);
                    ctx->conf[chn].roi_num++;
                }while(venc_get_curr_char(ctx) == ',');
                venc_find_next_char(ctx, ']');
                ctx->pos++; // skip ']'
            }
            else
            {
                venc_check_roi_config(ctx, chn, 0);
            }
        }
        else if(!strcmp(name, "entropy"))   venc_check_entropy_config(ctx, chn);
        else if(!strcmp(name, "ref"))   venc_check_ref_config(ctx, chn);
        else if(!strcmp(name, "crop"))   venc_check_crop_config(ctx, chn);
        else if(!strcmp(name, "slisplt"))   venc_check_slisplt_config(ctx, chn);
        else if(!strcmp(name, "intra"))
        {
            venc_find_next_char(ctx, ':');
            if(venc_get_next_char(ctx) == '[')
            {
                do{
                    ctx->pos++;
                    venc_check_intra_config(ctx, chn);
                }while(venc_get_curr_char(ctx) == ',');
                venc_find_next_char(ctx, ']');
                ctx->pos++; // skip ']'
            }
            else
            {
                venc_check_intra_config(ctx, chn);
            }
        }
        else if(!strcmp(name, "inter"))
        {
            venc_find_next_char(ctx, ':');
            if(venc_get_next_char(ctx) == '[')
            {
                do{
                    ctx->pos++;
                    venc_check_inter_config(ctx, chn);
                }while(venc_get_curr_char(ctx) == ',');
                venc_find_next_char(ctx, ']');
                ctx->pos++; // skip ']'
            }
            else
            {
                venc_check_inter_config(ctx, chn);
            }
        }
        else if(!strcmp(name, "trans"))   venc_check_trans_config(ctx, chn);
        else if(!strcmp(name, "vui"))
        {
            venc_find_next_char(ctx, ':');
            if(venc_get_next_char(ctx) == '[')
            {
                do{
                    ctx->pos++;
                    venc_check_vui_config(ctx, chn);
                }while(venc_get_curr_char(ctx) == ',');
                venc_find_next_char(ctx, ']');
                ctx->pos++; // skip ']'
            }
            else
            {
                venc_check_vui_config(ctx, chn);
            }
        }
        else if(!strcmp(name, "dblk"))
        {
            venc_find_next_char(ctx, ':');
            if(venc_get_next_char(ctx) == '[')
            {
                do{
                    ctx->pos++;
                    venc_check_dblk_config(ctx, chn);
                }while(venc_get_curr_char(ctx) == ',');
                venc_find_next_char(ctx, ']');
                ctx->pos++; // skip ']'
            }
            else
            {
                venc_check_dblk_config(ctx, chn);
            }
        }

        if(venc_get_curr_char(ctx) != ',')
        {
            break;
        }
        ctx->pos++; // skip ','
    }

    venc_find_next_char(ctx, '}'); //go to chn config end
    ctx->pos++; // skip '}'
    return 0;
}

int init_venc_config(VENC_Ctx_t *ctx)
{
    char name[MAX_JSON_STR_LEN];
    if(fgets(ctx->buff, sizeof(ctx->buff), ctx->fd) == NULL)
        return -1;
    ctx->pos = 0;

    venc_find_next_char(ctx, '{');
    ctx->pos++;
    venc_get_item_name(ctx, name, MAX_JSON_STR_LEN);

    if(!strcmp(name, "chn"))
    {
        venc_find_next_char(ctx, ':');
        if(venc_get_next_char(ctx) == '[')
        {
            do{
                ctx->pos++;
                venc_check_config_chn(ctx);
            }while(venc_get_next_char(ctx) == ',');
            venc_find_next_char(ctx, ']');
        }
        else
        {
            venc_check_config_chn(ctx);
        }
    }

    return 0;
}

int venc_read_config_file(VENC_Ctx_t *ctx)
{
    FILE *fd = fopen(VENC_CONFIG_PATH, "r");
    if(!fd)
    {
        venc_err("%s open failed.\n", VENC_CONFIG_PATH);
        return -1;
    }
    ctx->fd = fd;
    return 0;
}
#if 1
void venc_handle_sig(MI_S32 signo)
{
    if (signo == SIGINT)
    {
        venc_info("catch Ctrl + C, exit\n");
        g_Exit = 0;
    }
}
#endif

void venc_set_config(VENC_ChnConf_t *conf)
{
    MI_S32 s32Ret;

    if(conf->test_flag & VENC_TEST_INSERT)
    {
        s32Ret = MI_VENC_InsertUserData(conf->venc_chn, conf->ins_data, conf->ins_len);
        if(s32Ret)
            venc_err("MI_VENC_InsertUserData error, %X\n", s32Ret);
        else
            venc_info("MI_VENC_InsertUserData Success\n");
    }
    if(conf->test_flag & VENC_TEST_ROI)
    {
        int i = 0;
        for(i = 0; i < conf->roi_num && i < MAX_ROI_NUM; i++)
        {
            s32Ret = MI_VENC_SetRoiCfg(conf->venc_chn, &conf->roi[i]);
            if(s32Ret)
                venc_err("MI_VENC_SetRoiCfg %d error, %X\n", i, s32Ret);
            else
                venc_info("MI_VENC_SetRoiCfg %d Success\n", i);
        }
    }
    if(conf->test_flag & VENC_TEST_ENTROPY && conf->eModType == E_MI_VENC_MODTYPE_H264E)
    {
        s32Ret = MI_VENC_SetH264Entropy(conf->venc_chn, &conf->entropy);
        if(s32Ret)
            venc_err("MI_VENC_SetH264Entropy error, %X\n", s32Ret);
        else
            venc_info("MI_VENC_SetH264Entropy Success\n");
    }
    if(conf->test_flag & VENC_TEST_REF)
    {
        s32Ret = MI_VENC_SetRefParam(conf->venc_chn, &conf->ref);
        if(s32Ret)
            venc_err("MI_VENC_SetRefParam error, %X\n", s32Ret);
        else
            venc_info("MI_VENC_SetRefParam Success\n");
    }
    if(conf->test_flag & VENC_TEST_CROP)
    {
        s32Ret = MI_VENC_SetCrop(conf->venc_chn, &conf->crop);
        if(s32Ret)
            venc_err("MI_VENC_SetCrop error, %X\n", s32Ret);
        else
            venc_info("MI_VENC_SetCrop Success\n");
    }
    if(conf->test_flag & VENC_TEST_SLISPLT)
    {
        if(conf->eModType == E_MI_VENC_MODTYPE_H264E)
            s32Ret = MI_VENC_SetH264SliceSplit(conf->venc_chn, &conf->slisplt_h264);
        else if(conf->eModType == E_MI_VENC_MODTYPE_H265E)
            s32Ret = MI_VENC_SetH265SliceSplit(conf->venc_chn, &conf->slisplt_h265);
        else
            s32Ret = -1;
        if(s32Ret)
            venc_err("MI_VENC_SetH26xSliceSplit error, %X\n", s32Ret);
        else
            venc_info("MI_VENC_SetH26xSliceSplit Success\n");
    }
    if(conf->test_flag & VENC_TEST_INTRA)
    {
        if(conf->eModType == E_MI_VENC_MODTYPE_H264E)
            s32Ret = MI_VENC_SetH264IntraPred(conf->venc_chn, &conf->intra_h264);
        else if(conf->eModType == E_MI_VENC_MODTYPE_H265E)
            s32Ret = MI_VENC_SetH265IntraPred(conf->venc_chn, &conf->intra_h265);
        else
            s32Ret = -1;
        if(s32Ret)
            venc_err("MI_VENC_SetH26xIntraPred error, %X\n", s32Ret);
        else
            venc_info("MI_VENC_SetH26xIntraPred Success\n");
    }
    if(conf->test_flag & VENC_TEST_INTER)
    {
        if(conf->eModType == E_MI_VENC_MODTYPE_H264E)
            s32Ret = MI_VENC_SetH264InterPred(conf->venc_chn, &conf->inter_h264);
        else if(conf->eModType == E_MI_VENC_MODTYPE_H265E)
            s32Ret = MI_VENC_SetH265InterPred(conf->venc_chn, &conf->inter_h265);
        else
            s32Ret = -1;
        if(s32Ret)
            venc_err("MI_VENC_SetH26xInterPred error, %X\n", s32Ret);
        else
            venc_info("MI_VENC_SetH26xInterPred Success\n");
    }
    if(conf->test_flag & VENC_TEST_TRANS)
    {
        if(conf->eModType == E_MI_VENC_MODTYPE_H264E)
            s32Ret = MI_VENC_SetH264Trans(conf->venc_chn, &conf->trans_h264);
        else if(conf->eModType == E_MI_VENC_MODTYPE_H265E)
            s32Ret = MI_VENC_SetH265Trans(conf->venc_chn, &conf->trans_h265);
        else
            s32Ret = -1;
        if(s32Ret)
            venc_err("MI_VENC_SetH26xTrans error, %X\n", s32Ret);
        else
            venc_info("MI_VENC_SetH26xTrans Success\n");
    }
    if(conf->test_flag & VENC_TEST_VUI)
    {
        if(conf->eModType == E_MI_VENC_MODTYPE_H264E)
            s32Ret = MI_VENC_SetH264Vui(conf->venc_chn, &conf->vui_h264);
        else if(conf->eModType == E_MI_VENC_MODTYPE_H265E)
            s32Ret = MI_VENC_SetH265Vui(conf->venc_chn, &conf->vui_h265);
        else
            s32Ret = -1;
        if(s32Ret)
            venc_err("MI_VENC_SetH26xVui error, %X\n", s32Ret);
        else
            venc_info("MI_VENC_SetH26xVui Success\n");
    }
    if(conf->test_flag & VENC_TEST_DBLK)
    {
        if(conf->eModType == E_MI_VENC_MODTYPE_H264E)
            s32Ret = MI_VENC_SetH264Dblk(conf->venc_chn, &conf->dblk_h264);
        else if(conf->eModType == E_MI_VENC_MODTYPE_H265E)
            s32Ret = MI_VENC_SetH265Dblk(conf->venc_chn, &conf->dblk_h265);
        else
            s32Ret = -1;
        if(s32Ret)
            venc_err("MI_VENC_SetH26xDblk error, %X\n", s32Ret);
        else
            venc_info("MI_VENC_SetH26xDblk Success\n");
    }
}

void venc_output_thread(void *args)
{
    VENC_ChnConf_t *pConf = (VENC_ChnConf_t *)args;

    MI_SYS_ChnPort_t stVencChnInputPort;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32DevId = 0;
    MI_S32 vencFd = -1;
    fd_set read_fds;
    MI_VENC_Pack_t stPack[2];

    MI_VENC_GetChnDevid(pConf->venc_chn, &u32DevId);

    stVencChnInputPort.eModId = E_MI_MODULE_ID_VENC;
    stVencChnInputPort.u32DevId = u32DevId;
    stVencChnInputPort.u32ChnId = pConf->venc_chn;
    stVencChnInputPort.u32PortId = 0;

    //venc_info("create %s success\n", szFileName);

    vencFd = MI_VENC_GetFd(pConf->venc_chn);
    if(vencFd <= 0)
    {
        venc_err("Unable to get FD:%d for chn:%2d\n", vencFd, pConf->venc_chn);
        return;
    }
    else
    {
        venc_info("Venc Chn%2d FD:%d\n", pConf->venc_chn, vencFd);
    }

    while (g_Exit)
    {
        //printf("ctx->conf[0].yuv_width : %d \n", pConf->yuv_width);
        //printf("ctx->conf[0].yuv_height : %d \n", pConf->yuv_height);
        struct timeval TimeoutVal;
        MI_VENC_ChnStat_t stStat;
        MI_VENC_Stream_t stStream;
        int i;
        #if 1
        TimeoutVal.tv_sec  = 0;
        TimeoutVal.tv_usec = 200*1000;
        FD_ZERO(&read_fds);
        FD_SET(vencFd, &read_fds);
        s32Ret = select(vencFd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            venc_err("select failed\n");
            usleep(10 * 1000);
            continue;
        }
        else if (0 == s32Ret)
        {
            venc_info("select timeout\n");
            usleep(10 * 1000);
            continue;
        }
        else
        #endif
        {
            if (FD_ISSET(vencFd, &read_fds))
            {
                memset(&stStat, 0, sizeof(MI_VENC_ChnStat_t));
                s32Ret = MI_VENC_Query(pConf->venc_chn, &stStat);
                if (MI_SUCCESS != s32Ret || stStat.u32CurPacks == 0)
                {
                    venc_err("MI_VENC_Query error, %X\n", s32Ret);
                    usleep(10 * 1000);//sleep 10 ms
                    continue;
                }

                //printf("u32CurPacks:%d, u32LeftStreamFrames:%d\n", stStat.u32CurPacks, stStat.u32LeftStreamFrames);

                memset(&stStream, 0, sizeof(MI_VENC_Stream_t));
                memset(&stPack, 0, sizeof(MI_VENC_Pack_t)*2);

                stStream.u32PackCount = stStat.u32CurPacks;
                stStream.pstPack = stPack;
                s32Ret = MI_VENC_GetStream(pConf->venc_chn, &stStream, 40);
                if (MI_SUCCESS == s32Ret)
                {
                    //printf("u32PackCount:%d, u32Seq:%d, offset:%d, len:%d, type:%d, pts:0x%llx\n", stStream.u32PackCount, stStream.u32Seq,stStream.pstPack[0].u32Offset, stStream.pstPack[0].u32Len, stStream.pstPack[0].stDataType.eH264EType, stStream.pstPack[0].u64PTS);

                    if(pConf->eModType == E_MI_VENC_MODTYPE_H264E)
                    {
                        //printf("eRefType:%d\n", stStream.stH264Info.eRefType);
                    }
                    else if(pConf->eModType == E_MI_VENC_MODTYPE_H265E)
                    {
                        //printf("eRefType:%d\n", stStream.stH265Info.eRefType);
                    }

                    for (i = 0; i < stStream.u32PackCount; i ++)
                    {
                        //write(fd, stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset,stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset);
                        PutH264DataToBuffer((stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset), (stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset), stStream.pstPack[i].stDataType.eH264EType);
                        //printf("0");
                        usleep(100);
                    }

                    MI_VENC_ReleaseStream(pConf->venc_chn, &stStream);
                }
                else
                {
                    venc_err("MI_VENC_GetStream error, %X\n", s32Ret);
                    usleep(10 * 1000);//sleep 10 ms
                }
            }
        }
    }

    s32Ret = MI_VENC_CloseFd(pConf->venc_chn);
    if(s32Ret != 0)
    {
        venc_err("Chn%02d CloseFd error, Ret:%X\n", pConf->venc_chn, s32Ret);
    }
    printf("---------------- venc thread exit -------------");
    return 0;
}
#if 0
void *venc_input_thread(void *args)
{
    VENC_ChnConf_t *conf = (VENC_ChnConf_t *)args;

    MI_SYS_ChnPort_t stVencChnInputPort;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_BufInfo_t stBufInfo;
    FILE *pFile = NULL;
    MI_U32 u32FrameSize = 0;
    MI_U32 u32YSize = 0;
    MI_U32 u32FilePos = 0;
    struct stat st;
    MI_U32 u32DevId = 0;

    memset(&stVencChnInputPort, 0, sizeof(MI_SYS_ChnPort_t));
    MI_VENC_GetChnDevid(conf->venc_chn, &u32DevId);

    venc_info("chn:%d, dev:%d\n", conf->venc_chn, u32DevId);

    stVencChnInputPort.eModId = E_MI_MODULE_ID_VENC;
    stVencChnInputPort.u32DevId = u32DevId;
    stVencChnInputPort.u32ChnId = conf->venc_chn;
    stVencChnInputPort.u32PortId = 0;

    memset(&stBufConf, 0, sizeof(MI_SYS_BufConf_t));
    memset(&stBufConf, 0x0, sizeof(MI_SYS_BufConf_t));
    MI_SYS_GetCurPts(&stBufConf.u64TargetPts);
    stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stBufConf.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stBufConf.stFrameCfg.u16Width = conf->yuv_width;
    stBufConf.stFrameCfg.u16Height = conf->yuv_height;

    u32YSize = conf->yuv_width * conf->yuv_height;
    u32FrameSize = (u32YSize >> 1) + u32YSize;

    if ((s32Ret = stat(conf->yuv_path, &st)) != 0)
    {
        venc_err("stat %s error:%x\n", conf->yuv_path, s32Ret);
    }

    pFile = fopen(conf->yuv_path, "rb");
    if (pFile == NULL)
    {
        venc_err("open %s error\n", conf->yuv_path);
        return NULL;
    }

    if(st.st_size == 0)
    {
        st.st_size = u32FrameSize;
    }

    venc_info("open %s success, total size:%lld bytes\n", conf->yuv_path, (long long)st.st_size);

    venc_info("chn:%d u32YSize:%d,u32FrameSize:%d\n", stVencChnInputPort.u32ChnId,
        u32YSize, u32FrameSize);

    while (conf->bStart)
    {
        memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));

        u32FilePos = ftell(pFile);
        if ((st.st_size - u32FilePos) < u32FrameSize)
        {
            fseek(pFile, 0L, SEEK_SET);
        }

        s32Ret = MI_SYS_ChnInputPortGetBuf(&stVencChnInputPort, &stBufConf, &stBufInfo, &hHandle, 1000);
        if(MI_SUCCESS == s32Ret)
        {
            if (0 >= fread(stBufInfo.stFrameData.pVirAddr[0], 1, u32YSize, pFile))
            {
                fseek(pFile, 0, SEEK_SET);

                stBufInfo.bEndOfStream = TRUE;
                s32Ret = MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);
                if (MI_SUCCESS != s32Ret)
                {
                    venc_err("MI_SYS_ChnInputPortPutBuf error, %X\n", s32Ret);
                }
                continue;
            }

            if (0 >= fread(stBufInfo.stFrameData.pVirAddr[1], 1, u32YSize >> 1, pFile))
            {
                fseek(pFile, 0, SEEK_SET);

                stBufInfo.bEndOfStream = TRUE;
                s32Ret = MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);
                if (MI_SUCCESS != s32Ret)
                {
                    venc_err("MI_SYS_ChnInputPortPutBuf error, %X\n", s32Ret);
                }
                continue;
            }

            s32Ret = MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);
            if (MI_SUCCESS != s32Ret)
            {
                venc_err("MI_SYS_ChnInputPortPutBuf error, %X\n", s32Ret);
            }
        }
        else
        {
            venc_err("MI_SYS_ChnInputPortGetBuf error, chn:%d, %X\n", conf->venc_chn, s32Ret);
        }
    }

    fclose(pFile);

    return NULL;
}
#endif
int venc_start_chn(VENC_ChnConf_t *conf)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VENC_CHN VencChn = conf->venc_chn;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_SYS_ChnPort_t stVencChnOutputPort;

    memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));
    memset(&stVencChnOutputPort, 0, sizeof(MI_SYS_ChnPort_t));

    if (E_MI_VENC_MODTYPE_H264E == conf->eModType)
    {
        stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
        stChnAttr.stVeAttr.stAttrH264e.u32PicWidth = conf->pic_width;
        stChnAttr.stVeAttr.stAttrH264e.u32PicHeight = conf->pic_height;
        stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = conf->pic_width;
        stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = conf->pic_height;
        stChnAttr.stVeAttr.stAttrH264e.bByFrame = true;
        stChnAttr.stVeAttr.stAttrH264e.u32Profile = conf->profile;

        if(conf->eRcMode == E_MI_VENC_RC_MODE_H264FIXQP)
        {
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = conf->fps;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = conf->gop;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = conf->qp;
            stChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = conf->qp;
        }
        else if(conf->eRcMode == E_MI_VENC_RC_MODE_H264CBR)
        {
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264CBR;
            stChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate = conf->bitrate;
            stChnAttr.stRcAttr.stAttrH264Cbr.u32Gop = conf->gop;
            stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum = conf->fps;
            stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = 1;
        }
        else
        {
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264VBR;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32MaxBitRate = conf->bitrate;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32MaxQp = 48;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32MinQp = 12;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32Gop = conf->gop;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateNum = conf->fps;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateDen = 1;
        }
    }
    else if (E_MI_VENC_MODTYPE_H265E == conf->eModType)
    {
        stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
        stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = conf->pic_width;
        stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = conf->pic_height;
        stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = conf->pic_width;
        stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = conf->pic_height;
        stChnAttr.stVeAttr.stAttrH265e.bByFrame = true;

        if(conf->eRcMode == E_MI_VENC_RC_MODE_H265FIXQP)
        {
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265FIXQP;
            stChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateNum = conf->fps;
            stChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH265FixQp.u32Gop = conf->gop;
            stChnAttr.stRcAttr.stAttrH265FixQp.u32IQp = conf->qp;
            stChnAttr.stRcAttr.stAttrH265FixQp.u32PQp = conf->qp;
        }
        else if(conf->eRcMode == E_MI_VENC_RC_MODE_H265CBR)
        {
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265CBR;
            stChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = conf->bitrate;
            stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = conf->fps;
            stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH265Cbr.u32Gop = conf->gop;
        }
        else
        {
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265VBR;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate = conf->bitrate;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32Gop = conf->gop;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxQp = 48;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MinQp = 12;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum = conf->fps;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = 1;
        }
    }
    else if (E_MI_VENC_MODTYPE_JPEGE == conf->eModType)
    {
        stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;
        stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
        stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = conf->pic_width;
        stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = conf->pic_height;
        stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = conf->pic_width;
        stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = conf->pic_height;
        stChnAttr.stVeAttr.stAttrJpeg.bByFrame = true;
        stChnAttr.stRcAttr.stAttrMjpegFixQp.u32Qfactor = conf->qp;
    }

    s32Ret = MI_VENC_CreateChn(VencChn, &stChnAttr);
    if (MI_SUCCESS != s32Ret)
    {
        venc_err("MI_VENC_CreateChn %d error, %X\n", VencChn, s32Ret);
    }
    venc_info(" MI_VENC_CreateChn, vencChn:%d\n", VencChn);

    s32Ret = MI_VENC_SetMaxStreamCnt(VencChn, 4);
    if(MI_SUCCESS != s32Ret)
    {
        venc_err("MI_VENC_SetMaxStreamCnt %d error, %X\n", VencChn, s32Ret);
    }

    venc_set_config(conf);
    conf->vencFd = MI_VENC_GetFd(VencChn);
    venc_info("venc chn:%d\n", VencChn);

    // get es stream
    //pthread_create(&conf->pt_output, NULL, (void*)venc_output_thread, (void *)conf);
    // put yuv data to venc
    //pthread_create(&conf->pt_input, NULL, (void*)venc_input_thread, (void *)conf);


    // MI_VENC_InputSourceConfig_t stVenInSrc;
    // memset(&stVenInSrc, 0, sizeof(MI_VENC_InputSourceConfig_t));
    // stVenInSrc.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_RING_ONE_FRM;
    // s32Ret = MI_VENC_SetInputSourceConfig((MI_VENC_CHN)0, &stVenInSrc);
    // if (MI_SUCCESS != s32Ret)
    // {
    //     venc_err("MI_VENC_SetInputSourceConfig %d error, %X\n", VencChn, s32Ret);
    // }


    s32Ret = MI_VENC_StartRecvPic(VencChn);
    if (MI_SUCCESS != s32Ret)
    {
        venc_err("MI_VENC_StartRecvPic %d error, %X\n", VencChn, s32Ret);
    }

    if(0)
    {
        s32Ret = MI_VENC_SetRoiCfg(0, conf->roi);
        if(s32Ret)
            venc_err("MI_VENC_SetRoiCfg error, %X\n", s32Ret);
        else
            venc_info("MI_VENC_SetRoiCfg Success\n");
    }

    return 0;
}

int venc_end(void)
{
    MI_VENC_CHN VencChn = 0;
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret = MI_VENC_StopRecvPic(VencChn);
    if(s32Ret)
        venc_err("MI_VENC_StopRecvPic error, %X\n", s32Ret);
    else
        venc_info("MI_VENC_StopRecvPic Success\n");

    s32Ret = MI_VENC_DestroyChn(VencChn);
    if(s32Ret)
        venc_err("MI_VENC_DestroyChn error, %X\n", s32Ret);
    else
        venc_info("MI_VENC_DestroyChn Success\n");

    return 0;
}
int SensorId=0;
void Sensor_init()
{
    int id=0;
    int ret=-1;
    MI_U32 u32ResCount =0;
    MI_U8 u8ResIndex =0;
    MI_SNR_Res_t stRes;

    ret=MI_SNR_SetPlaneMode((MI_SNR_PAD_ID_e)id, FALSE); //HDR off
    if(ret !=0 )
    {
        printf("MI_SNR_SetPlaneMode is failed!!\n");
        return;
    }
    ret=MI_SNR_QueryResCount((MI_SNR_PAD_ID_e)SensorId, &u32ResCount);
    if(ret !=0 )
    {
        printf("MI_SNR_QueryResCount is failed!!\n");
        return;
    }
    for(u8ResIndex = 0; u8ResIndex < u32ResCount; u8ResIndex++)
    {
        MI_SNR_GetRes((MI_SNR_PAD_ID_e)0, u8ResIndex, &stRes);
        printf("SNR get Res index %d, Crop(%d,%d,%d,%d), outputsize(%d,%d), maxfps %d, minfps %d, ResDesc %s\n",
        u8ResIndex,
        stRes.stCropRect.u16X, stRes.stCropRect.u16Y, stRes.stCropRect.u16Width,stRes.stCropRect.u16Height,
        stRes.stOutputSize.u16Width, stRes.stOutputSize.u16Height,
        stRes.u32MaxFps,stRes.u32MinFps,
        stRes.strResDesc);
    }

    ret=MI_SNR_SetRes((MI_SNR_PAD_ID_e)SensorId, (MI_U32)0);
    if(ret !=0 )
    {
        printf("MI_SNR_SetRes is failed!!\n");
        return;
    }
    ret=MI_SNR_Enable((MI_SNR_PAD_ID_e)SensorId);
    if(ret !=0 )
    {
        printf("MI_SNR_Enable is failed!!\n");
        return;
    }
    printf("--------------sensor init is ok!!!!!----------\n");
    return;
}

void sensor_deinit()
{
    MI_SNR_Disable((MI_SNR_PAD_ID_e)SensorId);
    return;
}

int Vif_devId=0;
MI_VIF_DEV Vif_port=0;
void Vif_init()
{
    int ret=-1;
    MI_VIF_CHN chn=0;
    MI_SNR_PADInfo_t  stPad0Info;
    MI_SNR_PlaneInfo_t stSnrPlane0Info;
    MI_VIF_DevAttr_t stDevAttr;
    MI_VIF_ChnPortAttr_t stChnPortAttr;
    MI_SYS_PixelFormat_e ePixFormat;
    memset(&stPad0Info, 0x0, sizeof(MI_SNR_PADInfo_t));
    memset(&stSnrPlane0Info, 0x0, sizeof(MI_SNR_PlaneInfo_t));
    memset(&stDevAttr, 0, sizeof(MI_VIF_DevAttr_t));
    memset(&stChnPortAttr, 0, sizeof(MI_VIF_ChnPortAttr_t));

    ret=MI_SNR_GetPadInfo((MI_SNR_PAD_ID_e)SensorId, &stPad0Info);
    if(ret !=0 )
    {
        printf("MI_SNR_GetPadInfo is failed!!\n");
        return;
    }
    printf("eIntfMode=%d\n",stPad0Info.eIntfMode);
    ret=MI_SNR_GetPlaneInfo((MI_SNR_PAD_ID_e)SensorId, 0, &stSnrPlane0Info);
    if(ret !=0 )
    {
        printf("MI_SNR_GetPlaneInfo is failed!!\n");
        return;
    }
    ePixFormat = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stSnrPlane0Info.ePixPrecision, stSnrPlane0Info.eBayerId);
    printf("init vif u16Width=%d,u16Height=%d,x=%d,y=%d,ePixFormat=%#x\n",stSnrPlane0Info.stCapRect.u16Width,stSnrPlane0Info.stCapRect.u16Height,stSnrPlane0Info.stCapRect.u16X,stSnrPlane0Info.stCapRect.u16Y,ePixFormat);
    stDevAttr.eIntfMode = stPad0Info.eIntfMode;
    stDevAttr.eWorkMode = E_MI_VIF_WORK_MODE_RGB_REALTIME;
    stDevAttr.eHDRType = (MI_VIF_HDRType_e)0;
    ret=MI_VIF_SetDevAttr((MI_VIF_DEV)Vif_devId, &stDevAttr);
    if(ret !=0 )
    {
        printf("MI_VIF_SetDevAttr is failed!!\n");
        return;
    }
    ret=MI_VIF_EnableDev((MI_VIF_DEV)Vif_devId);
    if(ret !=0 )
    {
        printf("MI_VIF_EnableDev is failed!!\n");
        return;
    }
    stChnPortAttr.stCapRect.u16X = stSnrPlane0Info.stCapRect.u16X;
    stChnPortAttr.stCapRect.u16Y = stSnrPlane0Info.stCapRect.u16Y;
    stChnPortAttr.stCapRect.u16Width = stSnrPlane0Info.stCapRect.u16Width==0 ? 1920:stSnrPlane0Info.stCapRect.u16Width;
    stChnPortAttr.stCapRect.u16Height = stSnrPlane0Info.stCapRect.u16Height==0 ? 1080:stSnrPlane0Info.stCapRect.u16Height;
    stChnPortAttr.stDestSize.u16Width = stSnrPlane0Info.stCapRect.u16Width==0 ? 1920:stSnrPlane0Info.stCapRect.u16Width;
    stChnPortAttr.stDestSize.u16Height = stSnrPlane0Info.stCapRect.u16Height==0 ? 1080:stSnrPlane0Info.stCapRect.u16Height;
    //stChnPortAttr.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;//是这里有问题
    stChnPortAttr.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV422_UYVY;
    if(stPad0Info.eIntfMode == E_MI_VIF_MODE_BT656)
    {
        printf("MI_VIF_SetChnPortAttr eIntfMode is E_MI_VIF_MODE_BT656!!\n");
        stChnPortAttr.eFrameRate = E_MI_VIF_FRAMERATE_FULL;
        stChnPortAttr.eCapSel = E_MI_SYS_FIELDTYPE_BOTH;
        stChnPortAttr.eScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    }
    //stChnPortAttr.eFrameRate = E_MI_VIF_FRAMERATE_FULL;
    //stChnPortAttr.eCapSel = E_MI_SYS_FIELDTYPE_BOTH;
    printf("init vif u16Width=%d,u16Height=%d,w=%d,h=%d,pixFormat=yuv 420\n",stChnPortAttr.stCapRect.u16Width,stChnPortAttr.stCapRect.u16Height,stChnPortAttr.stDestSize.u16Width,stChnPortAttr.stDestSize.u16Height);
    ret=MI_VIF_SetChnPortAttr(chn, Vif_port, &stChnPortAttr);
    if(ret !=0 )
    {
        printf("MI_VIF_SetChnPortAttr is failed!! %#X\n",ret);
        return;
    }
    ret=MI_VIF_EnableChnPort(chn, Vif_port);
    if(ret !=0 )
    {
        printf("MI_VIF_EnableChnPort is failed!!\n");
        return;
    }
    printf("--------------vif init is ok!!!!!----------\n");
    return;
}
void Vif_deinit()
{
    MI_VIF_CHN chn=0;
    MI_VIF_DisableChnPort(chn, Vif_port);
    MI_VIF_DisableDev((MI_VIF_DEV)Vif_devId);
    return;
}

int VpePort=0;

void vpe_init()
{
    int ret=-1;
    MI_VPE_ChannelAttr_t stChannelVpeAttr;
    MI_VPE_ChannelPara_t stChannelVpeParam;
    MI_VPE_CHANNEL VpeChannel=0;
    MI_SYS_WindowRect_t stCrop;
    MI_VPE_PortMode_t stVpeMode;
    MI_SNR_PlaneInfo_t stSnrPlane0Info;
    MI_SNR_PADInfo_t  stPad0Info;
    MI_SYS_PixelFormat_e ePixFormat;
    MI_U32 u32CapWidth = 0, u32CapHeight = 0;

    MI_SNR_GetPadInfo((MI_SNR_PAD_ID_e)SensorId, &stPad0Info);
    MI_SNR_GetPlaneInfo((MI_SNR_PAD_ID_e)SensorId, 0, &stSnrPlane0Info);
    u32CapWidth = stSnrPlane0Info.stCapRect.u16Width==0 ? 1920:stSnrPlane0Info.stCapRect.u16Width;
    u32CapHeight = stSnrPlane0Info.stCapRect.u16Height==0 ? 1080:stSnrPlane0Info.stCapRect.u16Height;

    // if(stSnrPlane0Info.eBayerId == E_MI_SYS_PIXEL_BAYERID_MAX)
    // {
    //     ePixFormat = (int)stSnrPlane0Info.ePixel;
    // }
    // else
    // {
    //     ePixFormat = (int)RGB_BAYER_PIXEL(stSnrPlane0Info.ePixPrecision, stSnrPlane0Info.eBayerId);
    // }

    ePixFormat=(MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stSnrPlane0Info.ePixPrecision, stSnrPlane0Info.eBayerId);
    printf("init vpe get sensor u16Width=%d,u16Height=%d,x=%d,y=%d,ePixFormat=%#x\n",stSnrPlane0Info.stCapRect.u16Width,stSnrPlane0Info.stCapRect.u16Height,stSnrPlane0Info.stCapRect.u16X,stSnrPlane0Info.stCapRect.u16Y,ePixFormat);

    ret=MI_SNR_SetOrien(E_MI_SNR_PAD_ID_0, FALSE, FALSE);
    if(ret !=0 )
    {
        printf("MI_SNR_SetOrien is failed!!\n");
        return;
    }
    ret=MI_VPE_SetChannelRotation(0, E_MI_SYS_ROTATE_NONE);
    if(ret !=0 )
    {
        printf("MI_VPE_SetChannelRotation is failed!!\n");
        return;
    }
    memset(&stChannelVpeAttr, 0, sizeof(MI_VPE_ChannelAttr_t));
    memset(&stChannelVpeParam, 0x00, sizeof(MI_VPE_ChannelPara_t));
    stChannelVpeAttr.u16MaxW = u32CapWidth;
    stChannelVpeAttr.u16MaxH = u32CapHeight;
    stChannelVpeAttr.bNrEn= FALSE;
    stChannelVpeAttr.bEdgeEn= FALSE;
    stChannelVpeAttr.bEsEn= FALSE;
    stChannelVpeAttr.bContrastEn= FALSE;
    stChannelVpeAttr.bUvInvert= FALSE;
    stChannelVpeAttr.ePixFmt = E_MI_SYS_PIXEL_FRAME_YUV422_UYVY;
    stChannelVpeAttr.eRunningMode = E_MI_VPE_RUN_REALTIME_MODE;

    //stChannelVpeAttr.eHDRType  = 0;
    //stChannelVpeAttr.eSensorBindId = E_MI_VPE_SENSOR_INVALID; //E_MI_VPE_SENSOR0;
    stChannelVpeAttr.eSensorBindId = E_MI_VPE_SENSOR0;
    //stChannelVpeAttr.bRotation = FALSE;
    //stChannelVpeAttr.u32ChnPortMode = 0;//未知

    ret=MI_VPE_CreateChannel(VpeChannel, &stChannelVpeAttr);
    if(ret !=0 )
    {
        printf("MI_VPE_CreateChannel is failed!!ERROR %#x!!!!\n",ret);
        //return;
    }
    stChannelVpeParam.eHDRType = E_MI_VPE_HDR_TYPE_OFF;
    stChannelVpeParam.e3DNRLevel = E_MI_VPE_3DNR_LEVEL_OFF;
    ret=MI_VPE_SetChannelParam(VpeChannel, &stChannelVpeParam);
    if(ret !=0 )
    {
        printf("MI_VPE_SetChannelParam is failed!!\n");
        //return;
    }
    memset(&stCrop, 0, sizeof(MI_SYS_WindowRect_t));
    ret=MI_VPE_SetChannelCrop(VpeChannel, &stCrop);
    if(ret !=0 )
    {
        printf("MI_VPE_SetChannelCrop is failed!!\n");
        //return;
    }
    ret=MI_VPE_StartChannel (VpeChannel);
    if(ret !=0 )
    {
        printf("MI_VPE_StartChannel is failed!!\n");
        //return;
    }
    memset(&stVpeMode, 0, sizeof(stVpeMode));
    memset(&stCrop, 0, sizeof(MI_SYS_WindowRect_t));

    ret=MI_VPE_SetPortCrop(VpeChannel, VpePort, &stCrop);
    if(ret !=0 )
    {
        printf("MI_VPE_SetPortCrop is failed!!\n");
        //return;
    }
    ret=MI_VPE_GetPortMode(VpeChannel, VpePort, &stVpeMode);
    if(ret !=0 )
    {
        printf("MI_VPE_GetPortMode is failed!!\n");
        //return;
    }
    stVpeMode.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stVpeMode.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stVpeMode.u16Width = 1920;
    stVpeMode.u16Height= 1080;
    ret=MI_VPE_SetPortMode(VpeChannel, VpePort, &stVpeMode);
    if(ret !=0 )
    {
        printf("MI_VPE_SetPortMode is failed!!\n");
        //return;
    }
    ret=MI_VPE_EnablePort(VpeChannel, VpePort);
    if(ret !=0 )
    {
        printf("MI_VPE_EnablePort is failed!!\n");
        //return;
    }
    return;
}

void vpe_deinit()
{
    MI_VPE_CHANNEL VpeChannel=0;
    MI_VPE_DisablePort(0, VpePort);
    MI_VPE_StopChannel(VpeChannel);
    MI_VPE_DestroyChannel(VpeChannel);
    return;
}

void vif_bind_vpe()
{
    int ret=-1;
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = 0;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_REALTIME;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    ret=MI_SYS_BindChnPort2(&stBindInfo.stSrcChnPort, &stBindInfo.stDstChnPort,stBindInfo.u32SrcFrmrate, stBindInfo.u32DstFrmrate, stBindInfo.eBindType, stBindInfo.u32BindParam);
    if(ret !=0 )
    {
        printf("vif_bind_vpe MI_SYS_BindChnPort2 is failed!!\n");
        return;
    }
    return;
}

void vpe_bind_venc()
{
    int ret=-1;
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = 0;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    //stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_HW_RING;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    ret=MI_SYS_BindChnPort2(&stBindInfo.stSrcChnPort, &stBindInfo.stDstChnPort,stBindInfo.u32SrcFrmrate, stBindInfo.u32DstFrmrate, stBindInfo.eBindType, stBindInfo.u32BindParam);
    if(ret !=0 )
    {
        printf("vpe_bind_venc MI_SYS_BindChnPort2 is failed!!\n");
        return;
    }
    return;
}
extern int audio_thread();

static void init_pre_venc(void)
{
    Vif_init(); //
    vpe_init(); //
    vif_bind_vpe();
}

static int init_configs(void)
{
    int ret = 0;

    AppInitCfgInfoDefault();
    ret = AppInitCfgInfoFromFile();     //mode
    if (ret<0)
    {
        printf("ret = %d\n",ret);
        printf("build default config.conf \n");
        AppWriteCfgInfotoFile();
        system(CP_CONFIG_00);
        system(CP_CONFIG_01);
        system(CP_CONFIG_02);
        system(CP_CONFIG_03);
        system(CP_CONFIG_04);
        system(CP_CONFIG_05);
        system(CP_CONFIG_06);
        system(CP_CONFIG_07);
        system(CP_CONFIG_08);
        system(CP_CONFIG_09);
        system(CP_CONFIG_10);
    }
    else
    {
        printf("cfg get from file \n");
    }

    return 0;
}

static int init_system(void)
{
    ExecFunc(MI_SYS_Init(), MI_SUCCESS);
    Sensor_init();
    gpio_init();
    RingInit(); //video ring buffer init
    
    InitShareMem();
    init_configs();
    init_eth();

    init_pre_venc();
}

static deinit_pre_venc()
{
    Vif_deinit();
    vpe_deinit();
}



#if 1
int main(int argc, char **argv)
{
    pthread_t watchdog_handle = 0;
    pthread_t uart_watchdog_handle = 0;
    pthread_t rtp_sned_handle = 0;
    pthread_t audio_thread_handle = 0;
    pthread_t sharemem_handle = 0;
    pthread_t control_handle = 0;
    pthread_t backup_handle = 0;
    pthread_t gpio_handle = 0;
    pthread_t digit_led_handle;

    VENC_Ctx_t ctx; //
    memset(&ctx, 0, sizeof(VENC_Ctx_t));
    init_info();

    venc_parse_options(&ctx);

    struct sigaction sigAction;
    char szCmd[16];

    #if 0
    sigAction.sa_handler = venc_handle_sig;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = 0;
    sigaction(SIGINT, &sigAction, NULL);
    #endif

    if(venc_read_config_file(&ctx))
    {
        return -1;
    }
    
    init_system();
    init_venc_config(&ctx);

    //venc configure 
    venc_start_chn(ctx.conf);
    vpe_bind_venc();

#if 1
    CreateThread(&watchdog_handle, NULL, watchdog_main, NULL);
    CreateThread(&uart_watchdog_handle, NULL, uart_main, NULL);
    
    // get es stream
    CreateThread(&ctx.conf->pt_output, NULL, (void*)venc_output_thread, (void *)ctx.conf);
    CreateThread(&rtp_sned_handle, NULL, (void*)send_rtp_tx, NULL);
    CreateThread(&audio_thread_handle, NULL, (void*)audio_thread, NULL);
    CreateThread(&sharemem_handle, NULL, (void*)sharemem_main, NULL);
    
    CreateThread(&control_handle, NULL, (void*)control_slave, NULL);
    //CreateThread(&backup_handle, NULL, (void*)backup_host, NULL);
    #ifdef DIGIT_LED
    CreateThread(&digit_led_handle, NULL, digit_led_main, NULL);
    #endif

#endif
    while (g_Exit) //thread 0
    {
        sleep(1);
        g_HDMI_STATE = check_hdmi_signal(&h264_append_info);
        if (1 == g_HDMI_STATE)
        {   
            deinit_pre_venc();
            init_pre_venc();
        }
        //printf("\n\n");
    }

    pthread_join(sharemem_handle, NULL);
    pthread_join(ctx.conf[0].pt_output, NULL);
    pthread_join(audio_thread_handle, NULL);
    pthread_join(rtp_sned_handle, NULL);
    pthread_join(control_handle, NULL);
    //pthread_join(backup_handle, NULL);

    venc_end();
    vpe_deinit();
    Vif_deinit();
    sensor_deinit();
    ExecFunc(MI_SYS_Exit(), MI_SUCCESS);
    return 0;
}
#endif