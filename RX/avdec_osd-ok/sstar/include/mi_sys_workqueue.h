/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

 Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sigmastar Technology Corp. and be kept in strict confidence
(Sigmastar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sigmastar Confidential
Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifndef _MI_SYS_WORK_QUEUE_H_
#include "mi_common.h"
#include "mi_sys_datatype.h"
#include "cam_os_wrapper.h"


typedef void (*MisysWorkPoolCallback)(void *pData);

typedef enum
{
    eMisysWork_Invalid = 0,
    eMisysWork_Wait,
    eMisysWork_Cancel
}MisysWorkStatus_e;

typedef struct
{
    MisysWorkPoolCallback fpCB;
    void* pData;
    //void* pParam;
    MI_S32 s32Timeout;
    MisysWorkStatus_e eStatus;
}MisysWork_t;

typedef struct
{
    CamOsAtomic_t tNumWorks;
    int nWIdx;
    int nRIdx;
    CamOsTcond_t tTcond;
    CamOsThread tThread;
//    MisysWork_t tWorks[64];
    MisysWork_t *tWorks;
    MI_S32 s32MaxWorkCnt;
    MI_BOOL bInit;
}MisysWorkPool_t;

MI_S32 MisysWorkAdd(MisysWorkPool_t *ptWorkPool,MisysWorkPoolCallback fpCB,void* pData,MI_S32 pParam);
MI_S32 MisysWorkGet(MisysWorkPool_t *ptWorkPool,MisysWork_t *ptWork);
MI_BOOL MIsysWorkCancel(MisysWorkPool_t *ptWorkPool, void *pData);
void MisysWorkInit(MisysWorkPool_t* ptWorkPool, MI_S8 *pWqName, MI_S32 s32MaxWorkCnt);
void MisysWorkDenit(MisysWorkPool_t* ptWorkPool);
void MisysWorkInitEx(MisysWorkPool_t* ptWorkPool, MI_S8 *pWqName, MI_S32 s32MaxWorkCnt, void *(*fpWorkThreadCB)(void *), MI_S32 s32Priorty);
int queue_mi_work(MisysWorkPoolCallback fpCB,void* pData,MI_S32 pParam);

void MisysEarlyInit(void);
void MisysFinalRelease(void);
#endif

