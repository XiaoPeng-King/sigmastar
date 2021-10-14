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
#ifndef _MI_AIO_INTERNAL_H_
#define _MI_AIO_INTERNAL_H_

#define  USE_CAM_OS  (0)

//=============================================================================
// Include files
//=============================================================================
#include "AudioSRCProcess.h"
#include "AudioProcess.h"
#include "AudioAecProcess.h"
#include "AudioAedProcess.h"
#include "AudioSslProcess.h"
#include "AudioBfProcess.h"
#include "mi_aio_datatype.h"
#include "g711.h"
#include "g726.h"

//=============================================================================
// Extern definition
//=============================================================================

//=============================================================================
// Macro definition
//=============================================================================
#define MI_AUDIO_ALGORITHM_SAMPLES_UNIT   (128)
#define MI_AUDIO_VQE_SAMPLES_UNIT   (MI_AUDIO_ALGORITHM_SAMPLES_UNIT)

#define MI_AUDIO_USER_TRANS_BWIDTH_TO_BYTE(u32BitWidthByte, eWidth)          \
    switch(eWidth)  \
    {   \
        case E_MI_AUDIO_BIT_WIDTH_16:   \
            u32BitWidthByte = 2;    \
            break;  \
        case E_MI_AUDIO_BIT_WIDTH_24:   \
            u32BitWidthByte = 4;    \
            break;  \
        default:    \
            u32BitWidthByte = 0; \
            DBG_ERR("BitWidth is illegal = %u.\n", eWidth); \
            break; \
    }

#define MI_AUDIO_USER_TRANS_EMODE_TO_CHAN(u32Chan, eSoundmode) \
    switch(eSoundmode)  \
    {   \
        case E_MI_AUDIO_SOUND_MODE_MONO:        \
        case E_MI_AUDIO_SOUND_MODE_QUEUE:       \
            u32Chan = 1;    \
            break;      \
        case E_MI_AUDIO_SOUND_MODE_STEREO:   \
            u32Chan = 2;    \
            break;  \
        default:    \
            u32Chan = 0;    \
            DBG_ERR("eSoundmode is illegal = %u.\n", eSoundmode); \
            break; \
    }

#define MI_AUDIO_VQE_HPF_TRANS_TYPE(eHpfFreq, eIaaHpfFreq)          \
    switch(eHpfFreq)  \
    {   \
        case E_MI_AUDIO_HPF_FREQ_80:   \
            eIaaHpfFreq = AUDIO_HPF_FREQ_80;    \
            break;  \
        case E_MI_AUDIO_HPF_FREQ_120:   \
            eIaaHpfFreq = AUDIO_HPF_FREQ_120;    \
            break;  \
         case E_MI_AUDIO_HPF_FREQ_150:   \
            eIaaHpfFreq = AUDIO_HPF_FREQ_150; \
            break; \
        default:    \
            eIaaHpfFreq = AUDIO_HPF_FREQ_BUTT; \
            DBG_ERR("eHpfFreq is illegal = %d \n", eHpfFreq); \
            break; \
    }

#define MI_AUDIO_VQE_NR_SPEED_TRANS_TYPE(eNrSpeed, eIaaNrSpeed)\
    switch(eNrSpeed)\
    {\
        case E_MI_AUDIO_NR_SPEED_LOW:\
            eIaaNrSpeed = NR_SPEED_LOW;\
            break;\
        case E_MI_AUDIO_NR_SPEED_MID:\
            eIaaNrSpeed = NR_SPEED_MID;\
            break;\
        case E_MI_AUDIO_NR_SPEED_HIGH:\
            eIaaNrSpeed = NR_SPEED_HIGH;\
            break;\
        default:\
            DBG_ERR("eIaaNrSpeed is illegal %d\n", eNrSpeed);\
            break;\
    }

#define MI_AUDIO_VQE_SAMPLERATE_TRANS_TYPE(eSampleRate, eIaaSampleRate)\
    switch(eSampleRate)\
    {\
        case E_MI_AUDIO_SAMPLE_RATE_8000:\
			 eIaaSampleRate = IAA_APC_SAMPLE_RATE_8000;\
             break;\
        case E_MI_AUDIO_SAMPLE_RATE_16000:\
             eIaaSampleRate = IAA_APC_SAMPLE_RATE_16000;\
             break;\
        case E_MI_AUDIO_SAMPLE_RATE_48000:\
             eIaaSampleRate = IAA_APC_SAMPLE_RATE_48000;\
             break;\
        default:\
             DBG_ERR("eIaaSampleRate is illegal %d\n", eSampleRate);\
             break;\
    }

//=============================================================================
// Data type definition
//=============================================================================
typedef struct MI_AUDIO_AecLibInfo_s
{
    void *pAecLibHandle;
    unsigned int (*IaaAec_GetBufferSize)(void);
    AEC_HANDLE (*IaaAec_Init)(char* working_buffer_address, AudioAecInit * aec_init);
    int (*IaaAec_Config)(AEC_HANDLE handle, AudioAecConfig *aec_config);
    AEC_HANDLE (*IaaAec_Reset)(char* working_buffer_address, AudioAecInit * aec_init);
    int (*IaaAec_Free)(AEC_HANDLE handle);
    int (*IaaAec_Run)(AEC_HANDLE handle, short* pss_aduio_near_end, short* pss_aduio_far_end);
    int (*IaaAec_GetLibVersion)(unsigned short *ver_year,
    						 unsigned short *ver_date,
    						 unsigned short *ver_time);
    void (*IaaAec_GenKey)(char* code_out);
    void (*IaaAec_VerifyKey)(char* code_in);
    int (*IaaAec_GetKeyLen)(void);
} MI_AUDIO_AecLibInfo_t;

typedef struct MI_AUDIO_VqeLibInfo_s
{
    void *pApcLibHandle;
    unsigned int (*IaaApc_GetBufferSize)(AudioApcBufferConfig *apc_switch);
    APC_HANDLE (*IaaApc_Init)(char* const working_buffer_address,AudioProcessInit *audio_process_init, AudioApcBufferConfig *apc_switch);
    unsigned int (*IaaApc_Config)(APC_HANDLE handle,
                             AudioAnrConfig *anr_config,
                             AudioEqConfig *eq_config,
                             AudioHpfConfig *hpf_config,
                             AudioVadConfig *vad_config,
                             AudioDereverbConfig *dereverb_config,
                             AudioAgcConfig *agc_config);

    int (*IaaApc_Run)(APC_HANDLE,short* pss_audio_in);
    void (*IaaApc_Free)(APC_HANDLE);
    int (*IaaApc_GetLibVersion)(unsigned short *ver_year,
                             unsigned short *ver_date,
                             unsigned short *ver_time);

    int (*IaaApc_GetVadOut)(APC_HANDLE handle);
    unsigned int (*IaaApc_GetConfig)(APC_HANDLE handle,
                             AudioProcessInit *audio_process_init,
                             AudioAnrConfig *anr_config,
                             AudioEqConfig *eq_config,
                             AudioHpfConfig *hpf_config,
                             AudioVadConfig *vad_config,
                             AudioDereverbConfig *dereverb_config,
                             AudioAgcConfig *agc_config);
} MI_AUDIO_VqeLibInfo_t;

typedef struct MI_AUDIO_SrcLibInfo_s
{
    void *pSrcLibHandle;
    unsigned int (*IaaSrc_GetBufferSize)(SrcConversionMode mode);
    SRC_HANDLE (*IaaSrc_Init)(char *workingBufferAddress, SRCStructProcess *src_struct);
    int (*IaaSrc_Run)(SRC_HANDLE handle, short *audio_input, short *audio_output, int Npoints);
    int (*IaaSrc_Release)(SRC_HANDLE handle);
} MI_AUDIO_SrcLibInfo_t;

typedef struct MI_AUDIO_G711LibInfo_s
{
    void *pG711LibHandle;
    void (*G711Encoder)(short *pcm,unsigned char *code,int size,int lawflag);
    void (*G711Decoder)(short *pcm,unsigned char *code,int size,int lawflag);
    void (*G711Covert)(unsigned char *dst, unsigned char *src, int size, int flag);
} MI_AUDIO_G711LibInfo_t;

typedef struct MI_AUDIO_G726LibInfo_s
{
    void *pG726LibHandle;
    g726_state_t* (*g726_init)(g726_state_t *s, int bit_rate);
    int (*g726_decode)(g726_state_t *s, short amp[], const unsigned char g726_data[], int g726_bytes);
    int (*g726_encode)(g726_state_t *s, unsigned char g726_data[], const short amp[], int len);
} MI_AUDIO_G726LibInfo_t;

typedef struct MI_AUDIO_AedLibInfo_s
{
    void *pAedLibHandle;
    MI_AED_HANDLE (*MI_AED_Init)(AedParams *aed_params, int32_t *point_length);
    MI_AED_RET (*MI_AED_Uninit)(MI_AED_HANDLE handle);
    MI_AED_RET (*MI_AED_Run)(MI_AED_HANDLE handle, int16_t *audio_input);
    MI_AED_RET (*MI_AED_GetResult)(MI_AED_HANDLE handle);
    MI_AED_RET (*MI_AED_SetSensitivity)(MI_AED_HANDLE handle, AedSensitivity sensitivity);
    MI_AED_RET (*MI_AED_SetOperatingPoint)(MI_AED_HANDLE handle, int32_t operating_point);
    MI_AED_RET (*MI_AED_SetVadThreshold)(MI_AED_HANDLE handle, int32_t threshold_db);
    MI_AED_RET (*MI_AED_RunLsd)(MI_AED_HANDLE handle, int16_t *audio_input, int32_t agc_gain);
    MI_AED_RET (*MI_AED_SetLsdThreshold)(MI_AED_HANDLE handle, int32_t threshold_db);
    MI_AED_RET (*MI_AED_GetLsdResult)(MI_AED_HANDLE handle);
} MI_AUDIO_AedLibInfo_t;

typedef struct MI_AUDIO_SslLibInfo_s
{
	void *pSslLibHandle;
	unsigned int (*IaaSsl_GetBufferSize)(void);
	SSL_HANDLE (*IaaSsl_Init)(char* working_buffer,AudioSslInit* ssl_init);
	SSL_HANDLE (*IaaSsl_Config)(SSL_HANDLE handle,AudioSslConfig* ssl_config);
	int (*IaaSsl_Get_Config)(SSL_HANDLE handle,AudioSslConfig *ssl_config);
	int (*IaaSsl_Run)(SSL_HANDLE handle,short* microphone_input);
	int (*IaaSsl_Get_Direction)(SSL_HANDLE handle);
	SSL_HANDLE (*IaaSsl_Reset)(SSL_HANDLE working_buffer,AudioSslInit* ssl_init);
	int (*IaaSsl_Free)(SSL_HANDLE handle);
} MI_AUDIO_SslLibInfo_t;

typedef struct MI_AUDIO_BfLibInfo_s
{
	void *pBfLibHandle;
	unsigned int (*IaaBf_GetBufferSize)(void);
	BF_HANDLE (*IaaBf_Init)(char* working_buffer,AudioBfInit* bf_init);
	BF_HANDLE (*IaaBf_Config)(BF_HANDLE handle,AudioBfConfig* bf_config);
	int (*IaaBf_Get_Config)(BF_HANDLE handle,AudioBfConfig *bf_config);
	int (*IaaBf_Run)(BF_HANDLE handle,short* microphone_input,int * microphone_doa);
	BF_HANDLE (*IaaBf_Reset)(BF_HANDLE working_buffer,AudioBfInit* bf_init);
	int (*IaaBf_Free)(BF_HANDLE handle);
} MI_AUDIO_BfLibInfo_t;


//=============================================================================
// Variable definition
//=============================================================================


//=============================================================================
// Global function definition
//=============================================================================


#endif // _MI_AIO_INTERNAL_H_
