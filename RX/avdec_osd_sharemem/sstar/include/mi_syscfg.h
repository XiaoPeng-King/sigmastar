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
#ifndef _MI_SYSCFG_H_
#define _MI_SYSCFG_H_

#include "mi_syscfg_datatype.h"

MI_BOOL MI_SYSCFG_GetMmapInfo(const char *name, const MI_SYSCFG_MmapInfo_t **ppstMmap);
MI_BOOL MI_SYSCFG_GetMemoryInfo(const MI_SYSCFG_MemoryInfo_t **ppstMem);
MI_U8 MI_SYSCFG_GetDLCTableCount(void);
PanelInfo_t* MI_SYSCFG_GetPanelInfo(EN_DISPLAYTIMING_RES_TYPE eTiming, MI_BOOL bHdmiTx);

void MI_SYSCFG_SetupIniLoader(void);
void MI_SYSCFG_SetupMmapLoader(void);
void MI_SYSCFG_InitCmdqMmapInfo(void);
#endif
