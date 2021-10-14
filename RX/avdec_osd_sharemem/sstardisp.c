#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include "mi_sys.h"
#include "sstardisp.h"

#include "mi_panel_datatype.h"
#include "mi_panel.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
#if ENABLE_HDMI
#include "mi_hdmi.h"
#endif

#define USE_MIPI    0


#if defined(__cplusplus)||defined(c_plusplus)
extern "C"{
#endif
#if ENABLE_HDMI
static MI_S32 Hdmi_callback_impl(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_EventType_e Event, void *pEventParam, void *pUsrParam)
{
    switch (Event)
    {
        case E_MI_HDMI_EVENT_HOTPLUG:
            printf("E_MI_HDMI_EVENT_HOTPLUG.\n");
            break;
        case E_MI_HDMI_EVENT_NO_PLUG:
            printf("E_MI_HDMI_EVENT_NO_PLUG.\n");
            break;
        default:
            printf("Unsupport event.\n");
            break;
    }

    return MI_SUCCESS;
}
#endif
#define MAKE_YUYV_VALUE(y,u,v) ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK MAKE_YUYV_VALUE(0,128,128)

int sstar_disp_init(MI_DISP_PubAttr_t *pstDispPubAttr)
{
    MI_PANEL_LinkType_e eLinkType;
    MI_DISP_InputPortAttr_t stInputPortAttr;
#if ENABLE_HDMI
    MI_HDMI_InitParam_t stInitParam;
    MI_HDMI_Attr_t stAttr;
#endif
    MI_DISP_VideoLayerAttr_t stLayerAttr;

    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));

    MI_SYS_Init();

#if (!ENABLE_HDMI)

#else
    if (E_MI_DISP_INTF_HDMI == pstDispPubAttr->eIntfType)
    {
        stInitParam.pCallBackArgs = NULL;
        stInitParam.pfnHdmiEventCallback = Hdmi_callback_impl;
        MI_HDMI_Init(&stInitParam);
        MI_HDMI_Open(E_MI_HDMI_ID_0);

        memset(&stAttr, 0, sizeof(MI_HDMI_Attr_t));
        stAttr.stEnInfoFrame.bEnableAudInfoFrame  = FALSE;
        stAttr.stEnInfoFrame.bEnableAviInfoFrame  = FALSE;
        stAttr.stEnInfoFrame.bEnableSpdInfoFrame  = FALSE;
        stAttr.stAudioAttr.bEnableAudio = TRUE;
        stAttr.stAudioAttr.bIsMultiChannel = 0;
        stAttr.stAudioAttr.eBitDepth = E_MI_HDMI_BIT_DEPTH_16;
        stAttr.stAudioAttr.eCodeType = E_MI_HDMI_ACODE_PCM;
        stAttr.stAudioAttr.eSampleRate = E_MI_HDMI_AUDIO_SAMPLERATE_48K;
        stAttr.stVideoAttr.bEnableVideo = TRUE;
        stAttr.stVideoAttr.eColorType = E_MI_HDMI_COLOR_TYPE_RGB444;//default color type

#if ENABLE_HDMI_4K
        stAttr.stVideoAttr.eDeepColorMode = E_MI_HDMI_DEEP_COLOR_24BIT;
        stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_4K2K_30P;
#else
        stAttr.stVideoAttr.eDeepColorMode = E_MI_HDMI_DEEP_COLOR_MAX;
        stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_1080_60P;
#endif
        stAttr.stVideoAttr.eOutputMode = E_MI_HDMI_OUTPUT_MODE_HDMI;
        MI_HDMI_SetAttr(E_MI_HDMI_ID_0, &stAttr);
        MI_HDMI_Start(E_MI_HDMI_ID_0);
        pstDispPubAttr->u32BgColor = YUYV_BLACK;
#if ENABLE_HDMI_4K
        pstDispPubAttr->eIntfSync = E_MI_DISP_OUTPUT_3840x2160_30;
#else
        pstDispPubAttr->eIntfSync = E_MI_DISP_OUTPUT_1080P60;
#endif
        MI_DISP_SetPubAttr(0, pstDispPubAttr);

        MI_DISP_Enable(0);
        MI_DISP_BindVideoLayer(0, 0);
        memset(&stLayerAttr, 0, sizeof(stLayerAttr));

#if ENABLE_HDMI_4K
        if(bRota)
        {
            stLayerAttr.stVidLayerSize.u16Width  = 2160;
            stLayerAttr.stVidLayerSize.u16Height = 3840;
        }
        else
        {
            stLayerAttr.stVidLayerSize.u16Width  = 3840;
            stLayerAttr.stVidLayerSize.u16Height = 2160;
        }
#else
        if(bRota)
        {
            stLayerAttr.stVidLayerSize.u16Width  = OutDispHeight;
            stLayerAttr.stVidLayerSize.u16Height = OutDispWidth;
        }
        else
        {
            stLayerAttr.stVidLayerSize.u16Width  = OutDispWidth;
            stLayerAttr.stVidLayerSize.u16Height = OutDispHeight;
        }

#endif
        
        stLayerAttr.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420;
        stLayerAttr.stVidLayerDispWin.u16X      = OutX;
        stLayerAttr.stVidLayerDispWin.u16Y      = OutY;
        
#if ENABLE_HDMI_4K
        if(bRota)
        {
            stLayerAttr.stVidLayerDispWin.u16Width  = 2160;
            stLayerAttr.stVidLayerDispWin.u16Height = 3840;
        }
        else
        {
            stLayerAttr.stVidLayerDispWin.u16Width  = 3840;
            stLayerAttr.stVidLayerDispWin.u16Height = 2160;
        }
#else
        if(bRota)
        {
            stLayerAttr.stVidLayerDispWin.u16Width  = OutDispHeight;
            stLayerAttr.stVidLayerDispWin.u16Height = OutDispWidth;
        }
        else
        {
            stLayerAttr.stVidLayerDispWin.u16Width  = OutDispWidth;
            stLayerAttr.stVidLayerDispWin.u16Height = OutDispHeight;
        }
#endif
      
        MI_DISP_SetVideoLayerAttr(0, &stLayerAttr);
        MI_DISP_EnableVideoLayer(0);

        stInputPortAttr.u16SrcWidth = inDispWidth;
        stInputPortAttr.u16SrcHeight = inDispHeight;
        stInputPortAttr.stDispWin.u16X = OutX;
        stInputPortAttr.stDispWin.u16Y = OutY;
#if ENABLE_HDMI_4K
        if(bRota)
        {
            stInputPortAttr.stDispWin.u16Width = OutDispHeight;
            stInputPortAttr.stDispWin.u16Height = OutDispWidth;
        }
        else
        {
            stInputPortAttr.stDispWin.u16Width = OutDispWidth;
            stInputPortAttr.stDispWin.u16Height = OutDispHeight;
        }
#else
        if(bRota)
        {
            stInputPortAttr.stDispWin.u16Width = OutDispHeight;
            stInputPortAttr.stDispWin.u16Height = OutDispWidth;
        }
        else
        {
            stInputPortAttr.stDispWin.u16Width = OutDispWidth;
            stInputPortAttr.stDispWin.u16Height = OutDispHeight;
        }
#endif

        MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr);
        MI_DISP_EnableInputPort(0, 0);
        MI_DISP_SetInputPortSyncMode(0, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);
    }
#endif

    return 0;
}
int sstar_disp_Deinit(MI_DISP_PubAttr_t *pstDispPubAttr)
{

    MI_DISP_DisableInputPort(0, 0);
    MI_DISP_DisableVideoLayer(0);
    MI_DISP_UnBindVideoLayer(0, 0);
    MI_DISP_Disable(0);

    switch(pstDispPubAttr->eIntfType) {
#if ENABLE_HDMI
        case E_MI_DISP_INTF_HDMI:
            MI_HDMI_Stop(E_MI_HDMI_ID_0);
            MI_HDMI_Close(E_MI_HDMI_ID_0);
            MI_HDMI_DeInit();
            break;
#else
        case E_MI_DISP_INTF_VGA:
            break;

        case E_MI_DISP_INTF_LCD:
            MI_PANEL_DeInit();
            break;
#endif
        default: break;
    }

    MI_SYS_Exit();
    printf("sstar_disp_Deinit...\n");

    return 0;
}

#if defined(__cplusplus)||defined(c_plusplus)
}
#endif

