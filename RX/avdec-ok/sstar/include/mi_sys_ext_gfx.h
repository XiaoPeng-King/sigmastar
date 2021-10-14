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
#ifndef _MI_SYS_EXT_GFX_H_
#define  _MI_SYS_EXT_GFX_H_


#include "mi_gfx_datatype.h"

typedef struct mi_sys_ModuleDevGfxOps_s
{
    MI_S32 (*MI_GFX_OpenFunc)(void);
    MI_S32 (*MI_GFX_CloseFunc)(void);
    MI_S32 (*MI_GFX_WaitAllDoneFunc)(MI_BOOL bWaitAllDone, MI_U16 u16TargetFence);
    MI_S32 (*MI_GFX_QuickFillFunc)(MI_GFX_Surface_t *pstDst, MI_GFX_Rect_t *pstDstRect, \
            MI_U32 u32ColorVal, MI_U16 *pu16Fence);
    MI_S32 (*MI_GFX_GetAlphaThresholdValueFunc)(MI_U8 *pu8ThresholdValue);
    MI_S32 (*MI_GFX_SetAlphaThresholdValueFunc)(MI_U8 u8ThresholdValue);
    MI_S32 (*MI_GFX_BitBlitFunc)(MI_GFX_Surface_t *pstSrc, MI_GFX_Rect_t *pstSrcRect, \
            MI_GFX_Surface_t *pstDst,  MI_GFX_Rect_t *pstDstRect, MI_GFX_Opt_t *pstOpt, MI_U16 *pu16Fence);
    MI_S32 (*mi_gfx_PendingDoneFunc)(MI_U16 u16TargetFence);
} mi_sys_ModuleDevGfxOps_t;

#endif /// _MI_SYS_EXT_GFX_H_

