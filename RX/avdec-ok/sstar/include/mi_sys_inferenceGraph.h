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
#ifndef _MI_SYS_INFERENCEGRAPH_H_
#define _MI_SYS_INFERENCEGRAPH_H_
struct MI_SYS_ModDev_s;
struct MI_SYS_DevPass_s;
struct mi_sys_DevPassRTInfo_s;
struct MI_SYS_ChnPass_s;
struct mi_sys_InternalChnInputTaskInfo_s;
struct mi_sys_InternalChnOutputTaskInfo_s;
struct MI_SYS_InputPort_s;
struct MI_SYS_OutputPort_s;

typedef struct MI_SYS_DevPassInferDAG_Info_s
{
    MI_BOOL bFinalRenderRootFlag;

    MI_BOOL bUseDevForceSelfThreadDrive;
    //dummy register used to sync between cmdqs
    MI_U32  u32CrossCMDQSyncDummyRegAddr;
    MI_BOOL bFinalDelayInferenceNode;
    MI_BOOL bHasHWRingInInput;
    CamOsAtomic_t bindCnt;

// temporal variables
    struct CamOsListHead_t track_listhead;

    MI_BOOL bTmpDelayInferenceNode;
    MI_BOOL bTmpHasHWRingInInput;
    MI_BOOL bTmpRenderRootFlag;
    MI_BOOL bTmpDevPassRoleDetermined;
    MI_BOOL bHasDirectlyAfterHWRingBindShip;

    //temporal variable used usring spliting graph
    MI_S32 s32InferencedCnt;
    MI_S32  s32InferenceScore;
    MI_U32  u32InferencePhase;
} MI_SYS_DevPassInferDAG_Info_t;

typedef enum
{
    INFER_NONE,
    INFER_ASYNC,
    INFER_SYNC,
    INFER_HWRING
} INFER_TYPE;
typedef struct MI_SYS_InputPortInferDAG_Info_s
{
    INFER_TYPE eInferType;
    MI_U32 u32LastEnqueuedDMABasedBufPulseIndex;
} MI_SYS_InputPortInferDAG_Info_t;


#define MI_SYS_MAX_CHN_DYNAMIC_ACTION_REC_NUM 16
typedef struct MI_SYS_ChnInferAction_s
{
    MI_U32 u32InferSequenceNumber;
    const char *pstrActionName;
    MI_U32 u32InferFrameNumber;
    MI_U32 u32ContentPipeDelayInUS;
} MI_SYS_ChnInferAction_t;
typedef struct MI_SYS_ChnInferAction_Record_s
{
    MI_U32 u32InferPulseIndex;
    MI_U32 u32TotalWorkingMs;
    MI_U32 u32ISRCnt;
    MI_U32 u32CurInferRecordNum;
    MI_SYS_ChnInferAction_t stActionRecords[MI_SYS_MAX_CHN_DYNAMIC_ACTION_REC_NUM];
} MI_SYS_ChnInferAction_Record_t;
typedef struct MI_SYS_ChnPassInfDAG_Info_s
{
    struct CamOsListHead_t track_listhead;
    MI_U32 u32Phase;
    MI_U32 u32RootChnIdx;
    MI_SYS_ChnInferAction_Record_t stDynamicInferMainWorkerActionRec;
    MI_SYS_ChnInferAction_Record_t stDynamicInferAnsyncWorkerActionRec;
} MI_SYS_ChnPassInfDAG_Info_t;

void MI_SYS_Kickoff_RealTime_OutputTasks(struct mi_sys_DevPassRTInfo_s *pstRTInfo, struct mi_sys_InternalChnOutputTaskInfo_s *pstInternalOutputTask);
void MI_SYS_Kickoff_DMABasedLowLatency_OutputTasks(struct mi_sys_DevPassRTInfo_s *pstRTInfo, struct mi_sys_InternalChnOutputTaskInfo_s *pstInternalOutputTask);
void MI_SYS_InferGraph_Finish_Output_Task(struct mi_sys_DevPassRTInfo_s *pstRTInfo, struct mi_sys_InternalChnOutputTaskInfo_s *pstInternalOutputTask);
void MI_SYS_ProcessChnPassInqueueTasks(struct mi_sys_DevPassRTInfo_s *pstRTInfo, struct MI_SYS_ChnPass_s *pstChnPass);
void MI_SYS_InferGraph_InitDevPassDAGInfo(struct MI_SYS_DevPass_s *pstDevPass);
void MI_SYS_InferGraph_DeInitDevPassDAGInfo(struct MI_SYS_DevPass_s *pstDevPass);
void MI_SYS_InferGraph_InitChnPassDAGInfo(struct MI_SYS_ChnPass_s *pstChnPass);
void MI_SYS_InferGraph_DeInitChnPassDAGInfo(struct MI_SYS_ChnPass_s *pstChnPass);
void MI_SYS_InferGraph_InitInputPortDAGInfo(struct MI_SYS_InputPort_s *pstInputPort);
void MI_SYS_InferGraph_DeInitInputPortDAGInfo(struct MI_SYS_InputPort_s *pstInputPort);
void MI_SYS_InferGraph_OnBind(struct MI_SYS_InputPort_s *pstInputPort, struct MI_SYS_OutputPort_s *pstOutputPort);
void MI_SYS_InferGraph_OnUnBind(struct MI_SYS_InputPort_s *pstInputPort, struct MI_SYS_OutputPort_s *pstOutputPort);
void MI_SYS_InferGraph_OnUsrFifoDepthChanged(struct MI_SYS_OutputPort_s *pstOutputPort);
void MI_SYS_InferGraph_ReadLock(void);
void MI_SYS_InferGraph_ReadUnlock(void);
void MI_SYS_InferGraph_WriteLock(void);
void MI_SYS_InferGraph_WriteUnlock(void);
void MI_SYS_IMPL_InferGraphLockBindGraph(void);
void MI_SYS_IMPL_InferGraphUnlockBindGraph(void);
//void MI_SYS_InferGraph_FlushPeerRingModeTasks(struct MI_SYS_ChnPass_s  *pstChnPass);
MI_S32 MI_SYS_IMPL_EnsureInputPortFifoEmpty(struct MI_SYS_DevPass_s *pstDevPass, struct MI_SYS_InputPort_s *psNextInputPort , MI_BOOL bReadLock);

#if defined(MI_SYS_PROC_FS_DEBUG) && defined(__KERNEL__)
int MI_SYS_IMPL_InitInferGraphProcShow(struct seq_file *m, void *v);
#endif
MI_S32 _MI_SYS_IMPL_POS_IN_LIST(struct CamOsListHead_t *pos, struct CamOsListHead_t *plist_head);
void MI_SYS_InferGraph_RecAction(struct mi_sys_DevPassRTInfo_s *pstRTInfo, struct MI_SYS_ChnPass_s *pstChnPass, const char *pstrActionName, MI_U32 u32InferFrameNumber, MI_U32 u32PipelinelatencyInUS);
#endif
