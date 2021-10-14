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
#ifndef _MI_SYS_IMPL_H_
#define _MI_SYS_IMPL_H_

#include "mi_sys.h"
#include "mi_sys_internal.h"
#include "mi_sys_buf_mgr.h"
#include "mi_common_internal.h"
#include "mi_sys_proc_fs_internal.h"
#include "mi_sys_ext_gfx.h"
#include "mi_sys_inferenceGraph.h"
#include "mi_sys_mma_heap_impl.h"
#include "mi_sys_chunk_impl.h"
#include "mi_sys_cmdq.h"
#include "cam_os_wrapper.h"
#include "cam_os_util.h"
#include "cam_os_util_list.h"
#include "cam_os_util_bitmap.h"
#include "mi_sys_lock.h"

#define __MI_SYS_MAGIC_NUM__ 0x4D535953
#define __MI_SYS_DEVCHN_MAGIC_NUM__ 0x4D43484E
#define __MI_SYS_DEVPASS_MAGIC_NUM__ 0x4D504153
#define __MI_SYS_CHNPASS_MAGIC_NUM__ 0x4D434153
#define __MI_SYS_INPUTPORT_MAGIC_NUM__ 0x4D5F494E
#define __MI_SYS_OUTPORT_MAGIC_NUM__ 0x4D4F5554

#define MI_SYS_OUTPUT_BUF_CNT_QUOTA_DEFAULT (4)
#define MI_SYS_USR_INJECT_BUF_CNT_QUOTA_DEFAULT (4)


#define MI_PARAM_IGNORE     (0)
#define MI_SYS_FIRE_IMMEDIATELY  ((MI_S64)-1)

struct MI_SYS_OutputPort_s;
struct MI_SYS_ChnPass_s;
struct MI_SYS_DevChn_s;
struct MI_SYS_ModDev_s;

typedef struct MI_FrameRate_s
{
    MI_U32 u32Num;                 /* numerator of Framerate. */
    MI_U32 u32Den;                 /* denominator of Framerate. */
} MI_FrameRate_t;

typedef struct MI_SYS_InputPort_s
{
    MI_U32 u32MagicNumber;
    MI_BOOL bPortEnable;
    MI_U32 u32PortId;
    MI_U64 u64SidebandMsg;

    MI_FrameRate_t stSrcFrmrate;          /* Set Src Framerate */
    MI_FrameRate_t stDstFrmrate;          /* Set Dst Framerate */
    MI_FrameRate_t stRtSrcFrmrate;        /* Realtime Src Framerate */
    MI_U32 u32BufRcParamDispatch;
    MI_U32 u32BufRcParamAlloc;
    MI_U32 u32CurFrmCnt;
    MI_U64 u64TimeStamp;
    MI_U32 u32GetTotalCnt;
    MI_U32 u32GetOkCnt;
    MI_U32 u32FinishCnt;
    MI_U32 u32RewindCnt;

    CamOsAtomic_t usrLockedInjectBufCnt;
    CamOsAtomic_t usrInputPortBufQueueInjectBufCnt;
    CamOsAtomic_t usrInputPortBufQueuePipeInBufCnt;
    MI_BOOL bNeedLockBindIn;
    MI_BOOL bNeedLockInject;

    CamOsTcond_t inputBufCntWaitqueue;

    struct CamOsListHead_t stBindRelationShipList;

    MI_SYS_BindType_e eBindType;
    MI_BOOL bBindValid;
    mi_sys_PortBindCaps_t stBindCapability;
    struct MI_SYS_ChnPass_s *pstChnPass;
    struct MI_SYS_DevPass_s *pstDevPass;

    //could only be accessed under protection of MI_SYS_DevPass_t.input_pending_buflist_semlock;
    struct CamOsListHead_t cur_working_input_queue;
    struct MI_SYS_OutputPort_s *pstBindPeerOutputPort;
    struct mi_sys_Allocator_s *pstCusBufAllocator;
    struct mi_sys_Allocator_s *pstHWRingBufCusBufAllocator;
    //identify the milliseconds needed to be delayed before this port could start to process this pre-start task
    union
    {
        MI_U32    u32LowLatencyDelayMs;
        MI_U32    u32HWRingLineCnt;
    } stBindParam;
    MI_U32  u32LastStaticDelayTotalMS;
    MI_U32  u32LastStaticDelayTotalCnt;
    MI_U32 u32StaticDelayAccmMS;
    MI_U32 u32StaticDelayAccmCnt;
    MI_SYS_FrameBufExtraConfig_t stInputBufExtraConf;

    //Clear Whole Padding flag.
    MI_BOOL bClearWholeBuf;

    MI_U8 u8PortBufName[TASK_COMM_LEN];
    MI_SYS_InputPortInferDAG_Info_t stInferDAGInfo;

    struct MI_SYS_BufferQueue_s stDumpInjectBufQueue;
    struct MI_SYS_BufferQueue_s stDumpBindInBufQueue;
} MI_SYS_InputPort_t;

typedef struct MI_SYS_OutputPort_s
{
    MI_U32 u32MagicNumber;
    MI_BOOL bPortEnable;
    MI_U32 u32PortId;
    MI_U32 u32UsrFifoCount;
    //driver back reference back depth
    MI_U32 u32DrvBkRefFifoDepth;

    MI_U32 u32OutputPortBufCntQuota;
    MI_BOOL bUserFIFOBlockMode;

    CamOsAtomic_t usrLockedBufCnt;
    CamOsAtomic_t totalOutputPortInUsedBuf;

    MI_FrameRate_t stRtDstFrmrate;        /* Realtime Dest Framerate */
    MI_U32 u32CurFrmCnt;
    MI_U64 u64TimeStamp;
    MI_U32 u32GetTotalCnt;
    MI_U32 u32GetOkCnt;
    MI_U32 u32FinishCnt;
    MI_U32 u32RewindCnt;

    //for special driver which need to refer back pre-processed buffer
    struct MI_SYS_BufferQueue_s stDrvBkRefFifoQueue;
    struct MI_SYS_BufferQueue_s stUsrGetFifoBufQueue;
    MI_U64 u64UsrGetFifoBufQueueAddCnt;
    MI_U64 u64UsrGetFifoBufQueueDiscardCnt;
//    struct MI_SYS_BufferQueue_s stWorkingQueue;

    struct mi_sys_Allocator_s *pstCusBufAllocator;
    struct mi_sys_Allocator_s *pstOutputBufAllocator;

    struct MI_SYS_ChnPass_s *pstChnPass;
    struct MI_SYS_DevPass_s *pstDevPass;

    CamOsTsem_t stBindedInputListSemlock;
    struct CamOsListHead_t stBindPeerInputPortList;
    ///For GetFd Start
    CamOsTsem_t stPollSemlock;
    MI_COMMON_PollFile_t *pstPollFile;
    //output buffer sequence number
    MI_U32 u32SequenceNumber;
    ///TODO:RM stMTtestChnPort
    MI_SYS_ChnPort_t stMTtestChnPort;
    ///For GetFd End
    MI_SYS_FrameBufExtraConfig_t stOutputBufExtraRequirements;

    //Clear Whole Padding flag.
    MI_BOOL bClearWholeBuf;

    MI_U8 u8PortBufName[TASK_COMM_LEN];
} MI_SYS_OutputPort_t;

typedef enum
{
    MI_SYS_CHNOPS_PREPROCESS_INPUTTASK,
    MI_SYS_CHNOPS_ENQUEUE_INPUTTASK,
    MI_SYS_CHNOPS_BARRIER_INPUTTASK,
    MI_SYS_CHNOPS_CHECK_INPUTTASK,
    MI_SYS_CHNOPS_DEQUEUE_INPUTTASK,

    MI_SYS_CHNOPS_POLL_ASYNC_OUTPUTTASK,
    MI_SYS_CHNOPS_ENQUEUE_ASYNC_OUTPUTTASK,
    MI_SYS_CHNOPS_BARRIER_ASYNC_OUTPUTTASK,
    MI_SYS_CHNOPS_CHECK_OUTPUTTASK,
    MI_SYS_CHNOPS_DEQUEUE_OUTPUTTASK,
    MI_SYS_CHNOPS_MAX
} MI_SYS_CHNPASS_OPS_TYPE;
typedef struct MI_SYS_ChnPass_StaticInfo_s
{
    MI_U32 u32LastUpdateFPSJiffies[MI_SYS_CHNOPS_MAX];
    MI_U32 u32LastUpdatedFPS[MI_SYS_CHNOPS_MAX];
    MI_U32 u32LastUpdatedVariance[MI_SYS_CHNOPS_MAX];
    MI_U32 u32LastUpdatedAverage[MI_SYS_CHNOPS_MAX];
    MI_U32 u32LastUpdatedMaxVal[MI_SYS_CHNOPS_MAX];
    MI_U32 u32CurAccumVariance[MI_SYS_CHNOPS_MAX];
    MI_U32 u32CurAccumFPS[MI_SYS_CHNOPS_MAX];
    MI_U32 u32AccumMaxVal[MI_SYS_CHNOPS_MAX];
    MI_U32 u32CurAccumTimeInNS[MI_SYS_CHNOPS_MAX];
} MI_SYS_ChnPass_StaticInfo_t;

typedef struct MI_SYS_ChnPass_s
{
    unsigned int u32MagicNumber;
    //MI_BOOL bPassPersistentSkipped;
    MI_S32 s32PassId;
    MI_U32 u32InputPortNum;
    MI_U32 u32OutputPortNum;
    struct MI_SYS_DevChn_s *pstDevChn;
    struct MI_SYS_DevPass_s *pstDevPass;
    MI_SYS_InputPort_t *pstInputPorts;//MI_SYS_MAX_INPUT_PORT_CNT
    MI_SYS_OutputPort_t *pstOutputPorts;//MI_SYS_MAX_OUTPUT_PORT_CNT
    MI_SYS_ChnPassInfDAG_Info_t stInferDAGInfo;
    MI_U32 u32LastOutputAllocFailedWarningJiffies;
    MI_U32 u32LastSuccessPreProcessInputTaskJiffies;
    MI_SYS_ChnPass_StaticInfo_t stChnPassPerfStatics;
} MI_SYS_ChnPass_t;

typedef struct MI_SYS_DevChn_s
{
    unsigned int u32MagicNumber;
    MI_BOOL bChnEnable;
    MI_U32 u32ChannelId;
    char *pu8MMAHeapName;
    CamOsAtomic_t chnIsrDone;
    struct MI_SYS_Allocator_Collection_s stAllocatorCollection;//should remove
    struct MI_SYS_ModDev_s *pstModDev;
    MI_SYS_ChnPass_t  astChnPass[MI_SYS_MAX_DEV_PASS_CNT];
    MI_S32 s32Pid;
    CamOsAtomic_t s32ChnUsdCurrentBufSize;
    CamOsAtomic_t s32ChnUsdPeakBufSize;
} MI_SYS_DevChn_t;
#define INVALID_INFER_PULSE_INDEX ((MI_U32)-1)
#define NEXT_INFER_PULSE_INDEX(val) ((((val)+1)==INVALID_INFER_PULSE_INDEX)?((val)+1):((val)+2))
static inline MI_BOOL MI_SYS_IMPL_PULSEINDEX_IsGE(MI_U32 u32FirstIndex, MI_U32 u32SecondIndex)
{
    if (u32FirstIndex >= u32SecondIndex)
        return (u32FirstIndex - u32SecondIndex)<=0x7FFFFFFFUL;
    return (u32FirstIndex+(0xFFFFFFFFUL-u32SecondIndex)+1)<=0x7FFFFFFFUL;
}

typedef struct mi_sys_DevPassRTInfo_s
{
    struct mi_sys_CmdQ_s *pCmdInf;

    MI_SYS_DEV_PASS_HANDLE pstBarriedDevPass;
    struct CamOsListHead_t list_of_later_barrier_devpass;
    struct CamOsListHead_t stDMABasedLowLatencyOutputTaskPendingList;//todo modify name

    //for remember later render devpass
    MI_BOOL bRenderDelayProcDevPassPhase;
    struct MI_SYS_DevPass_s *pstDelayProcDevPass;
    MI_U32 u32InferPulseIndex;
    MI_U32 u32AsyncOutputAccumFrameNumber;
    MI_U32 u32InferSequenceNumber;
    MI_BOOL bMainWorkerThread;
    char comm_name[TASK_COMM_LEN];
} mi_sys_DevPassRTInfo_t;

typedef enum
{
    MI_SYS_DEVOPS_ASYNC_PULSE_KICKOFF_DELAY,
    MI_SYS_DEVOPS_KICKOFF_INTERVAL,
    MI_SYS_DEVOPS_NEXTQUEUE_PENDING_DELAY,
    MI_SYS_DEVOPS_MAX
} MI_SYS_DEVPASS_OPS_TYPE;
typedef struct MI_SYS_DEVPass_StaticInfo_s
{
    MI_U32 u32LastUpdateFPSJiffies[MI_SYS_DEVOPS_MAX];
    MI_U32 u32LastUpdatedFPS[MI_SYS_DEVOPS_MAX];
    MI_U32 u32LastUpdatedVariance[MI_SYS_DEVOPS_MAX];
    MI_U32 u32LastUpdatedAverage[MI_SYS_DEVOPS_MAX];
    MI_U32 u32LastUpdatedMaxVal[MI_SYS_DEVOPS_MAX];
    MI_U32 u32CurAccumVariance[MI_SYS_DEVOPS_MAX];
    MI_U32 u32CurAccumFPS[MI_SYS_DEVOPS_MAX];
    MI_U32 u32AccumMaxVal[MI_SYS_DEVOPS_MAX];
    MI_U32 u32CurAccumTimeInNS[MI_SYS_CHNOPS_MAX];
} MI_SYS_DEVPass_StaticInfo_t;

typedef struct MI_SYS_DevPass_s
{
    MI_U32 u32MagicNumber;//must be __MI_SYS_DEVPASS_MAGIC_NUM__
    struct MI_SYS_ModDev_s *pstModDev;
    MI_S32 s32PassId;
    mi_sys_DevPassOpsInfo_t stPassOpsInfo;
    MI_U32 u32InputPortNum;
    MI_U32 u32OutputPortNum;

    CamOsThread main_worker_thread_handle;
    CamOsThread async_output_thread_handle;

    CamOsTcond_t input_thread_waitqueue;
    CamOsTcond_t output_thread_waitqueue;
    CamOsTcond_t input_buf_monitor_waitqueue;

    CamOsTsem_t input_task_semlock;
    struct CamOsListHead_t input_working_tasklist;
    CamOsTsem_t lazzy_rewind_inputtask_semlock;
    struct CamOsListHead_t lazzy_rewind_inputtask_list;

    CamOsTsem_t output_task_semlock;
    struct CamOsListHead_t output_working_tasklist;
    struct CamOsListHead_t output_finished_tasklist;

    CamOsTsem_t input_workingqueue_semlock;
    //current pulse working lst list
    struct CamOsListHead_t cur_working_input_queue;
    //next pulse fifo list, should not being processed until next pulse kickoff
    CamOsTsem_t new_pulse_fifo_inputqueque_semlock;
    struct CamOsListHead_t new_pulse_fifo_inputqueue;
    CamOsTsem_t next_todo_pulse_inputqueque_semlock;
    struct CamOsListHead_t next_todo_pulse_inputqueue;
    MI_U32 u32NextToDoQueueSequenceNumber;

    struct CamOsListHead_t listhead_in_later_barrier_list;//list head used in pstLockerRTInfo->stInjectLockedPassList;

    unsigned long last_inputbuf_process_tick;//used to rotect
    MI_S64  last_async_wakeup_event_NS;//used to rotect
    struct mi_sys_CmdQ_s *pstLastCmdQ;
    MI_SYS_DevPassInferDAG_Info_t stInferDAGInfo;
    MI_SYS_ChnPass_t *pstLastNeedToBarrierChnPass;
    struct mi_sys_DevPassRTInfo_s *pstLoggedRT;
    MI_U32 u32LastBarrierInferFrameNumber;
    MI_S64 s64LastTaskBufContentTimeStampInNS;
    MI_U32 u32ISRCnt;
    MI_U32 bNewInputEnqueueEvent;
    MI_BOOL bInternalInputDequeueEvent;
    MI_U32 u32WorkerThreadStartJiffies;
    MI_S64 s64LastKickoffTimeStamp;
    MI_S64 s64LastNextToDoQueueApplyTimeStamp;
    MI_SYS_DEVPass_StaticInfo_t stDevPassPerfStatics;
} MI_SYS_DevPass_t;

typedef struct MI_SYS_ModDev_s
{
    MI_U32 u32MagicNumber;
    MI_ModuleId_e eModuleId;
    MI_U32 u32DevId;
    MI_U32 u32DevPassNum;
    MI_U32 u32DevChnNum;
    MI_U32 u32DevInputPortNum;
    MI_U32 u32DevOutputPortNum;
    MI_BOOL bEnableMmu;
    MI_BOOL bEnableChnIsrCheck;
    MI_U32 u32ThreadPriority;  /* From 1(lowest) to 99(highest), use OS default priority if set 0 */

    CamOsTcond_t inputWaitqueue;
    struct CamOsListHead_t listModDev;
    struct MI_SYS_Allocator_Collection_s stAllocatorCollection;//remove
    void *pUsrData;

    mi_sys_ModuleDevOps_t astModuleDevOps;
    MI_SYS_DevPass_t astDevPass[MI_SYS_MAX_DEV_PASS_CNT];
    ///astChannels must set to last position
    struct MI_SYS_DevChn_s astDevChn[0];
} MI_SYS_ModDev_t;

typedef struct mi_sys_InternalChnOutputTaskInfo_s
{
    struct CamOsListHead_t list_in_work_queue;
    MI_SYS_DevPass_t *pstDevPass;//parent Devpass
    MI_SYS_ChnPass_t *pstChnPass;//parent Channel Pass
    uint32_t u32EqueueJiffies;
    uint32_t u32LastWarningJiffies;
    MI_U32 u32InferFrameNumber;
    MI_S64 s64TaskBufContentTimeStampInNS;
    MI_U16 u16SWKickOffFence;
    struct mi_sys_CmdQ_s *pKickoffCmdInf;
    mi_sys_ChnOutputTaskInfo_t stOutputTaskInfo;
} mi_sys_InternalChnOutputTaskInfo_t;

typedef struct mi_sys_InternalChnInputTaskInfo_s
{
    struct CamOsListHead_t list_in_work_queue;
    MI_SYS_DevPass_t *pstDevPass;//parent pass
    MI_SYS_ChnPass_t *pstChnPass;//parent Channel Pass
    MI_U64  u64InputTaskPTS;
    uint32_t u32EqueueJiffies;
    uint32_t u32LastWarningJiffies;
    MI_U32 u32InferFrameNumber;
    MI_S64 s64TaskBufContentTimeStampInNS;
    MI_U16 u16SWKickOffFence;
    struct mi_sys_CmdQ_s *pKickoffCmdInf;
    mi_sys_ChnInputTaskInfo_t stInputTaskInfo;
} mi_sys_InternalChnInputTaskInfo_t;

typedef enum
{
    E_MI_SYS_IDR_BUF_TYPE_INPUT_PORT = 0,
    E_MI_SYS_IDR_BUF_TYPE_OUTPUT_PORT,
    E_MI_SYS_IDR_BUF_TYPE_MMAP_TO_USER_SPACE,
    E_MI_SYS_IDR_BUF_TYPE_USER_MMA_ALLOC,
    E_MI_SYS_IDR_BUF_TYPE_USER_PRIVATE_POOL_ALLOC,
} MI_SYS_IDR_BUF_TYPE_e;

typedef struct MI_SYS_BufHandleIdrData_s
{
    struct CamOsListHead_t list;
    MI_SYS_IDR_BUF_TYPE_e eBufType;
    MI_SYS_BufRef_t *pstBufRef;
    union
    {
        MI_SYS_ChnPort_t stChnPort;
        MI_SYS_DRV_HANDLE miSysDrvHandle;
    };
    MI_U32 u32ExtraFlags;
    MI_S32 s32Pid;
    MI_PHY phyAddr;
} MI_SYS_BufHandleIdrData_t;

typedef struct MI_SYS_DevChnPrivateMMAHeap_s
{
    MI_U32 u32Devid;
    MI_S32 s32ChnId;
    mi_sys_chunk_mgr_t privheap_chunk_mgr;
    MI_PHY u64DevPrivHeapBasePA;
    struct CamOsListHead_t list_dev_chn_heap;
}MI_SYS_DevChnPrivateMMAHeap_t;

typedef struct MI_SYS_ChnPortOutputPrivateMMAHeap_s
{
    MI_U32 u32Devid;
    MI_U32 u32ChnId;
    MI_U32 u32Port;
    mi_sys_mma_allocator_t *pstChnPortBufBufMmaAllocator;
    struct CamOsListHead_t list_dev_chn_port_buf_heap;
}MI_SYS_ChnPortOutputPrivateMMAHeap_t;

typedef struct MI_SYS_MmuMmapKernerVirAddrHashMap_s
{
    struct CamOsHListNode_t   hentry;
    unsigned long u32KerVirAddr;
    void *pData;
}MI_SYS_MmuMmapKernerVirAddrHashMap_t;


MI_S32 MI_SYS_IMPL_Init(void);

MI_S32 MI_SYS_IMPL_InitEarly(void);

MI_S32 MI_SYS_IMPL_Exit(void);

MI_S32 MI_SYS_IMPL_ExitSYSForce(void);

MI_S32 MI_SYS_IMPL_BindChnPort(MI_SYS_ChnPort_t *pstSrcChnPort, MI_SYS_ChnPort_t *pstDstChnPort, MI_U32 u32SrcFrmrate,  MI_U32 u32DstFrmrate);
MI_S32 MI_SYS_IMPL_BindChnPort2(MI_SYS_ChnPort_t *pstSrcChnPort, MI_SYS_ChnPort_t *pstDstChnPort,
        MI_U32 u32SrcFrmrate,  MI_U32 u32DstFrmrate, MI_SYS_BindType_e eBindType, MI_U32 u32BindParam);

MI_S32 MI_SYS_IMPL_UnBindChnPort(MI_SYS_ChnPort_t *pstSrcChnPort, MI_SYS_ChnPort_t *pstDstChnPort);

MI_S32 MI_SYS_IMPL_GetBindbyDest (MI_SYS_ChnPort_t *pstDstChnPort, MI_SYS_ChnPort_t *pstSrcChnPort);

MI_S32 MI_SYS_IMPL_GetVersion(MI_SYS_Version_t *pstVersion);

MI_S32 MI_SYS_IMPL_GetCurPts(MI_U64 *pu64Pts);

MI_S32 MI_SYS_IMPL_InitPtsBase(MI_U64 u64PtsBase);

MI_S32 MI_SYS_IMPL_SyncPts(MI_U64 u64Pts);

MI_S32 MI_SYS_IMPL_Mmap(MI_PHY phyAddr, MI_U32 u32Size, void **ppVirtualAddress, MI_BOOL bCaChe);

MI_S32 MI_SYS_IMPL_Munmap(void *pVirtualAddress, MI_U32 u32Size);
MI_S32 MI_SYS_IMPL_FlushInvCache(void * va, MI_U32 u32Size);

MI_S32 MI_SYS_IMPL_SetReg(MI_U32 u32RegAddr, MI_U16 u16Value, MI_U16 u16Mask);

MI_S32 MI_SYS_IMPL_GetReg(MI_U32 u32RegAddr, MI_U16 *pu16Value);

MI_S32 MI_SYS_IMPL_SetChnMMAConf( MI_ModuleId_e eModId,  MI_U32  u32DevId, MI_U32 u32ChnId,MI_U8 *pu8MMAHeapName);

MI_S32 MI_SYS_IMPL_GetChnMMAConf( MI_ModuleId_e eModId,  MI_U32  u32DevId, MI_U32 u32ChnId,void  *data, MI_U32 u32Length);

MI_S32 MI_SYS_IMPL_ChnInputPortGetBuf(MI_SYS_ChnPort_t *pstChnPort,MI_SYS_BufConf_t *pstBufConf, MI_SYS_BufInfo_t *pstBufInfo, MI_SYS_BUF_HANDLE *BufHandle, MI_S32 s32TimeOutMs, MI_U32 u32ExtraFlags);

MI_S32 MI_SYS_IMPL_ChnInputPortPutBuf(MI_SYS_BUF_HANDLE BufHandle, MI_SYS_BufInfo_t *pstBufInfo, MI_BOOL bDropBuf);

MI_S32 MI_SYS_IMPL_ChnOutputPortGetBuf(MI_SYS_ChnPort_t *pstChnPort,MI_SYS_BufInfo_t *pstBufInfo, MI_SYS_BUF_HANDLE *BufHandle,MI_U32 u32ExtraFlags);

MI_S32 MI_SYS_IMPL_ChnOutputPortPutBuf(MI_SYS_BUF_HANDLE BufHandle);

MI_S32 MI_SYS_IMPL_InjectBuf(MI_SYS_BUF_HANDLE handle, MI_SYS_ChnPort_t *pstChnInputPort);

MI_S32 MI_SYS_IMPL_SetChnOutputPortDepth(MI_SYS_ChnPort_t *pstChnPort, MI_U32 u32UserFrameDepth, MI_U32 u32BufQueueDepth);

MI_S32 MI_SYS_IMPL_GetChnOutputPortDepth(MI_SYS_ChnPort_t *pstChnPort, MI_U32 *pu32UserFrameDepth, MI_U32 *pu32BufQueueDepth);

MI_SYS_DRV_HANDLE MI_SYS_IMPL_RegisterDev(mi_sys_ModuleDevInfo_t *pstMouleInfo, void *pUsrData
#ifdef MI_SYS_PROC_FS_DEBUG
        ,mi_sys_ModuleDevProcfsOps_t *pstModuleProcfsOps
        ,struct proc_dir_entry *proc_dir_entry
#endif
                                         );
MI_S32 MI_SYS_IMPL_UnRegisterDev(MI_SYS_DRV_HANDLE miSysDrvHandle);

#ifdef MI_SYS_PROC_FS_DEBUG
MI_S32 MI_SYS_IMPL_RegistCommand(MI_U8 *u8Cmd, MI_U8 u8MaxPara,
                                 MI_S32 (*fpExecCmd)(MI_SYS_DEBUG_HANDLE_t,MI_U32,MI_U8,MI_U8 **,void *),
                                 MI_SYS_DRV_HANDLE hHandle);
#endif

MI_S32 MI_SYS_IMPL_GetChnBufInfo(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PassId ,mi_sys_ChnBufInfo_t *pstChnBufInfo);

MI_S32 MI_SYS_IMPL_PrepareTaskOutputBuf(mi_sys_ChnTaskInfo_t *pstTask);

MI_S32 MI_SYS_IMPL_EnableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId);
MI_S32 MI_SYS_IMPL_DisableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId);

MI_S32 MI_SYS_IMPL_EnableOutputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId, MI_U32 u32PortId);
MI_S32 MI_SYS_IMPL_DisableOutputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId, MI_U32 u32PortId);
MI_S32 MI_SYS_IMPL_SetOutputPortBufExtConf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId, MI_U32 u32PortId, MI_SYS_FrameBufExtraConfig_t *pstBufExtraConf);
MI_S32 MI_SYS_IMPL_SetInputPortBufExtConf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId,MI_U32 u32PortId, MI_SYS_FrameBufExtraConfig_t *pstBufExtraConf);
MI_S32 MI_SYS_IMPL_SetOutputPortBufClearConf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId,MI_U32 u32PortId, MI_BOOL bClear);
MI_S32 MI_SYS_IMPL_SetInputPortBufClearConf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId,MI_U32 u32PortId, MI_BOOL bClear);

MI_S32 MI_SYS_IMPL_SetOutputPortUserFIFOBlockMode(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId, MI_BOOL mode);

MI_S32 MI_SYS_IMPL_EnableInputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId, MI_U32 u32PortId);
MI_S32 MI_SYS_IMPL_DisableInputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId, MI_U32 u32PortId);
MI_S32 MI_SYS_IMPL_SetInputPortSidebandMsg(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId, MI_U32 u32PortId, MI_U64 u64SidebandMsg);

MI_S32 MI_SYS_IMPL_SetReg (MI_U32 u32RegAddr, MI_U16 u16Value, MI_U16 u16Mask);
MI_S32 MI_SYS_IMPL_GetReg (MI_U32 u32RegAddr, MI_U16 *pu16Value);

MI_S32 MI_SYS_IMPL_MmaAlloc(MI_U8 *u8MMAHeapName, MI_U8 *u8BufName, MI_U32 u32blkSize ,MI_PHY *phyAddr);
MI_S32 MI_SYS_IMPL_MmaAlloc_Timeout(MI_U8 *u8MMAHeapName, MI_U8 *u8BufName, MI_U32 u32blkSize ,MI_PHY *phyAddr, MI_U32 u32TimeOut);

MI_S32 MI_SYS_IMPL_MmaFree(MI_PHY phyAddr);

void * MI_SYS_IMPL_Vmap(MI_PHY u64PhyAddr, MI_U32 u32Size, MI_BOOL bCache);
void MI_SYS_IMPL_UnVmap(void *pVirtAddr);

MI_S32 MI_SYS_IMPL_ConfDevPubPools(MI_ModuleId_e  eModule, MI_U32 u32DevId,   MI_VB_PoolListConf_t  stPoolListConf);
MI_S32 MI_SYS_IMPL_RelDevPubPools(MI_ModuleId_e  eModule, MI_U32 u32DevId);
MI_S32 MI_SYS_IMPL_ConfGloPubPools(MI_VB_PoolListConf_t  stPoolListConf);
MI_S32 MI_SYS_IMPL_RelGloPubPools(void);
void MI_SYS_IMPL_FlushInputPortTasks(MI_SYS_InputPort_t *pstNextInputPort, const char *caller);
//void MI_SYS_IMPL_FlushOutputPortTasks(MI_SYS_OutputPort_t *pstCurOutputPort, const char *caller);


struct mi_sys_Allocator_s;

MI_S32 MI_SYS_IMPL_SetOutputPortUsrAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32OutputportId, mi_sys_Allocator_t *pstUserAllocator);
mi_sys_Allocator_t *MI_SYS_IMPL_SysmemAllocatorGet(void);

MI_S32 MI_SYS_IMPL_SetInputPortUsrAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId, MI_U32 u32PortId, struct mi_sys_Allocator_s *pstUserAllocator);
MI_S32 MI_SYS_IMPL_SetPeerOutputPortCusAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId, MI_U32 u32PortId, struct mi_sys_Allocator_s *pstUserAllocator);
MI_S32 MI_SYS_IMPL_EnsureOutportBKRefFifoDepth(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId, MI_U32 u32PortId, MI_U32 u32BkRefFifoDepth);

MI_S32 MI_SYS_IMPL_MmapBufToMultiUserVirAddr(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_SYS_BufInfo_t *pstBufInfo, void **ppBufHandle, MI_BOOL bCache);
MI_S32 MI_SYS_IMPL_UnmapBufToMultiUserVirAddr(MI_SYS_DRV_HANDLE miSysDrvHandle, void *pBufHandle, MI_SYS_BufInfo_t **ppstBufInfo, MI_BOOL bFlushCache);

MI_PHY MI_SYS_IMPL_Cpu2Miu_BusAddr(MI_PHY cpu_addr);
MI_PHY MI_SYS_IMPL_Miu2Cpu_BusAddr(MI_PHY miu_phy_addr);
MI_SYS_BufferAllocation_t* MI_SYS_IMPL_AllocMMALowLevelBufAllocation(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_SYS_BufConf_t *pstBufConfig);
MI_SYS_BufInfo_t* MI_SYS_IMPL_InnerAllocBufFromVbPool(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32blkSize, MI_PHY *phyAddr);
MI_S32 MI_SYS_IMPL_FreeInnerVbPool(MI_SYS_BufInfo_t *stBufInfo);
MI_S32 MI_SYS_IMPL_ConfDevPubPools(MI_ModuleId_e  eModule, MI_U32 u32DevId, MI_VB_PoolListConf_t  stPoolListConf);
MI_S32 MI_SYS_IMPL_RelDevPubPools(MI_ModuleId_e  eModule, MI_U32 u32DevId);
MI_S32 MI_SYS_IMPL_Set_VDEC_VBPool_Flag(MI_U32 VdecVbPoolFlag);
MI_U32 MI_SYS_IMPL_Get_VDEC_VBPool_Flag(void);

MI_U32 MI_SYS_IMPL_Gfx_Ops_Callback(mi_sys_ModuleDevGfxOps_t *ops);
#if LINUX_VERSION_CODE == KERNEL_VERSION(4,9,84)
MI_S32 MI_SYS_IMPL_MemsetPa(MI_PHY pa, MI_U32 u32Val, MI_U32 u32Lenth);
MI_S32 MI_SYS_IMPL_MemcpyPa(MI_PHY paDst, MI_PHY paSrc, MI_U32 u32Lenth);
MI_S32 MI_SYS_IMPL_BufFillPa(MI_SYS_FrameData_t *pstBuf, MI_U32 u32Val, MI_SYS_WindowRect_t *pstRect);
MI_S32 MI_SYS_IMPL_BufBlitPa(MI_SYS_FrameData_t *pstDstBuf, MI_SYS_WindowRect_t *pstDstRect, MI_SYS_FrameData_t *pstSrcBuf, MI_SYS_WindowRect_t *pstSrcRect);
MI_S32 MI_SYS_IMPL_Read_Uuid(MI_U64 *u64Uuid);
#endif

int _MI_SYS_IMPL_RcAcceptBufDispatch(MI_SYS_InputPort_t *pstCurInputPort);
void _MI_SYS_IMPL_StepRcBufDispatch(MI_SYS_InputPort_t *pstCurInputPort);
int _MI_SYS_IMPL_RcAcceptBufAlloc(MI_SYS_InputPort_t *pstCurInputPort);
void _MI_SYS_IMPL_StepRcBufAlloc(MI_SYS_InputPort_t *pstCurInputPort);
void _MI_SYS_IMPL_CalcInputPortFrameRate(MI_SYS_InputPort_t *pstCurInputPort);
void _MI_SYS_IMPL_CalcOutputPortFrameRate(MI_SYS_OutputPort_t *pstCurOutputPort);

MI_S32 MI_SYS_IMPL_EnqueueBufToInputPort(mi_sys_DevPassRTInfo_t *pstRTInfo, MI_SYS_BufRef_t *pstBufRef,
                                    MI_SYS_InputPort_t *pstCurInputPort, MI_SYS_OutputPort_t *pstPearOutputPort, MI_BOOL bFromUsrInject);
//if u32ChannelId not equal to -1, we only process target channel ID
MI_BOOL MI_IMPL_KickOff_SyncInferenced_DevPassInputBufs(mi_sys_DevPassRTInfo_t *pstRTInfo, MI_SYS_DevPass_t *pstDevPass, MI_U32 u32TargetChannelId);
void MI_IMPL_Poll_PulseBufUsed(mi_sys_DevPassRTInfo_t *pstRTInfo, MI_SYS_InputPort_t *psNextInputPort);
MI_BOOL bInternalBusExchgmode(MI_SYS_BindType_e eBindType);
MI_BOOL bLowLatencyKickOff(MI_SYS_BindType_e eBindType);
MI_BOOL bRecurKickOffNextStage(MI_SYS_BindType_e eBindType);
void MI_SYS_IMPL_NotifyWakeupISREvent(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_S32 s32PassId , MI_S32 s32ChnId);
void MI_SYS_IMPL_DequeueInputOutputTasks(mi_sys_DevPassRTInfo_t *pstRTInfo, MI_SYS_DevPass_t *pstDevPass);
void MI_SYS_IMPL_FinishOutputTasks(mi_sys_DevPassRTInfo_t *pstRTInfo, MI_SYS_DevPass_t *pstDevPass, MI_U32 u32TargetChannelId);
void _MI_SYS_IMPL_ModuleIdToPrefixName(MI_ModuleId_e eModuleId, char *prefix_name);

MI_U32 MI_SYS_IMPL_GetNewPulseFifoInputQueueCnt(MI_SYS_InputPort_t *pstInputPort);
MI_U32 MI_SYS_IMPL_GetNextTodoPulseFifoInputQueueCnt(MI_SYS_InputPort_t *pstInputPort);
MI_U32 MI_SYS_IMPL_GetCurWorkingFifoInputQueueCnt(MI_SYS_InputPort_t *pstInputPort);
MI_U32 MI_SYS_IMPL_GetWorkingInputTaskCnt(MI_SYS_InputPort_t *pstInputPort);
MI_U32 MI_SYS_IMPL_GetLazzyRewindInputTaskCnt(MI_SYS_InputPort_t *pstInputPort);
MI_U32 MI_SYS_IMPL_GetWorkingOutputTaskCnt(MI_SYS_OutputPort_t *pstOutputPort);
MI_U32 MI_SYS_IMPL_GetFinishedOutputTaskCnt(MI_SYS_OutputPort_t *pstOutputPort);
MI_S32 MI_SYS_IMPL_GetPollData(MI_SYS_ChnPort_t *pstChnPort, void **pPrivate);
MI_S32 MI_SYS_IMPL_PutPollData(MI_S32 s32Fd);
MI_S32 MI_SYS_IMPL_ConfigPrivateMMAPool(MI_SYS_GlobalPrivPoolConfig_t *pstGlobalPrivPoolConf);
MI_S32 MI_SYS_IMPL_PrivateDevChnHeapAlloc(MI_ModuleId_e eModule , MI_U32 u32Devid , MI_S32 s32ChnId, MI_U8 *u8BufName, MI_U32 u32blkSize ,MI_PHY *phyAddr ,MI_BOOL bTailAlloc);
MI_S32 MI_SYS_IMPL_PrivateDevChnHeapFree(MI_ModuleId_e eModule , MI_U32 u32Devid , MI_S32 s32ChnId, MI_PHY phyAddr);
MI_S32 MI_SYS_IMPL_IsMmuMma(MI_BOOL *pbMmuMma);
MI_S32 MI_SYS_IMPL_FreeUsrProcessBuf(MI_S32 s32Pid);
MI_S32 MI_SYS_IMPL_MmaAlloc_ByChn(MI_U8 *u8MMAHeapName, MI_U8 *u8BufName, MI_U32 u32blkSize,MI_PHY *phyAddr , void *pPrivate);


#if (defined __KERNEL__) && (defined CONFIG_MI_SYS_ENABLE_DETECT_DEADLOCK)

#define MI_SYS_OUTPORT_BINDED_INPUT_LIST_SEMLOCK_DOWN(pstOutputPort) \
    do{ \
        char szLockName[MI_SYS_LOCK_NAME_LENGTH] = {'\0'}; \
        snprintf(szLockName, MI_SYS_LOCK_NAME_LENGTH, "mod%u_dev%u_pass%u_chn%u_port%u_stBindedInputListSemlock", \
                        pstOutputPort->pstDevPass->pstModDev->eModuleId, pstOutputPort->pstDevPass->pstModDev->u32DevId, \
                        pstOutputPort->pstDevPass->s32PassId, pstOutputPort->pstChnPass->pstDevChn->u32ChannelId, \
                        pstOutputPort->u32PortId); \
        MI_SYS_DOWN(&pstOutputPort->stBindedInputListSemlock, szLockName); \
      }while(0)

#define MI_SYS_OUTPORT_BINDED_INPUT_LIST_SEMLOCK_UP(pstOutputPort) \
    do{ \
        char szLockName[MI_SYS_LOCK_NAME_LENGTH] = {'\0'}; \
        snprintf(szLockName, MI_SYS_LOCK_NAME_LENGTH, "mod%u_dev%u_pass%u_chn%u_port%u_stBindedInputListSemlock", \
                        pstOutputPort->pstDevPass->pstModDev->eModuleId, pstOutputPort->pstDevPass->pstModDev->u32DevId, \
                        pstOutputPort->pstDevPass->s32PassId, pstOutputPort->pstChnPass->pstDevChn->u32ChannelId, \
                        pstOutputPort->u32PortId); \
        MI_SYS_UP(&pstOutputPort->stBindedInputListSemlock, szLockName); \
      }while(0)

#define MI_SYS_GLOBAL_DEV_LIST_SEMLOCK_DOWN() \
    do{ \
        MI_SYS_DOWN(&mi_sys_global_dev_list_semlock, "mi_sys_global_dev_list_semlock"); \
      }while(0)

#define MI_SYS_GLOBAL_DEV_LIST_SEMLOCK_UP() \
    do{ \
        MI_SYS_UP(&mi_sys_global_dev_list_semlock, "mi_sys_global_dev_list_semlock"); \
      }while(0)

#define MI_SYS_GLOBAL_DEV_LAST_CMDQ_SEMLOCK_DOWN() \
    do{ \
        MI_SYS_DOWN(&mi_sys_global_devLastCmdqLock, "mi_sys_global_devLastCmdqLock"); \
      }while(0)

#define MI_SYS_GLOBAL_DEV_LAST_CMDQ_SEMLOCK_UP() \
    do{ \
        MI_SYS_UP(&mi_sys_global_devLastCmdqLock, "mi_sys_global_devLastCmdqLock"); \
      }while(0)

#else

#define MI_SYS_OUTPORT_BINDED_INPUT_LIST_SEMLOCK_DOWN(pstOutputPort) \
    do{ \
        MI_SYS_DOWN(&pstOutputPort->stBindedInputListSemlock, NULL); \
      }while(0)

#define MI_SYS_OUTPORT_BINDED_INPUT_LIST_SEMLOCK_UP(pstOutputPort) \
    do{ \
        MI_SYS_UP(&pstOutputPort->stBindedInputListSemlock, NULL); \
      }while(0)

#define MI_SYS_GLOBAL_DEV_LIST_SEMLOCK_DOWN() \
    do{ \
        MI_SYS_DOWN(&mi_sys_global_dev_list_semlock, NULL); \
      }while(0)

#define MI_SYS_GLOBAL_DEV_LIST_SEMLOCK_UP() \
    do{ \
        MI_SYS_UP(&mi_sys_global_dev_list_semlock, NULL); \
      }while(0)

#define MI_SYS_GLOBAL_DEV_LAST_CMDQ_SEMLOCK_DOWN() \
    do{ \
        MI_SYS_DOWN(&mi_sys_global_devLastCmdqLock, NULL); \
      }while(0)

#define MI_SYS_GLOBAL_DEV_LAST_CMDQ_SEMLOCK_UP() \
    do{ \
        MI_SYS_UP(&mi_sys_global_devLastCmdqLock, NULL); \
      }while(0)

#endif/*#if (defined __KERNEL__) && (defined CONFIG_MI_SYS_ENABLE_DETECT_DEADLOCK)*/

#define MI_SYS_ABC(x) (((x) >= 0) ? (x) : -(x))
#define MI_SYS_MAX(a,b) (((a) >= (b)) ? (a) : (b))

static inline void  _MI_SYS_IMPL_StaticChnPassPerf(MI_SYS_ChnPass_t *pstChnPass, MI_SYS_CHNPASS_OPS_TYPE eOps, MI_U32 u32TimeCostInNs)
{
    MI_SYS_BUG_ON(eOps>=MI_SYS_CHNOPS_MAX);
    MI_SYS_BUG_ON(!pstChnPass || pstChnPass->u32MagicNumber!=__MI_SYS_CHNPASS_MAGIC_NUM__);

    pstChnPass->stChnPassPerfStatics.u32CurAccumFPS[eOps]++;
    pstChnPass->stChnPassPerfStatics.u32CurAccumVariance[eOps] += MI_SYS_ABC((MI_S32)u32TimeCostInNs-(MI_S32)pstChnPass->stChnPassPerfStatics.u32LastUpdatedAverage[eOps]);
    pstChnPass->stChnPassPerfStatics.u32AccumMaxVal[eOps] = MI_SYS_MAX(pstChnPass->stChnPassPerfStatics.u32AccumMaxVal[eOps], u32TimeCostInNs);
    pstChnPass->stChnPassPerfStatics.u32CurAccumTimeInNS[eOps] += u32TimeCostInNs;
    if(_mi_time_diff(pstChnPass->stChnPassPerfStatics.u32LastUpdateFPSJiffies[eOps], jiffies)>=HZ)
    {
        pstChnPass->stChnPassPerfStatics.u32LastUpdatedFPS[eOps] = pstChnPass->stChnPassPerfStatics.u32CurAccumFPS[eOps];
        if(pstChnPass->stChnPassPerfStatics.u32CurAccumFPS[eOps])
        {
            pstChnPass->stChnPassPerfStatics.u32LastUpdatedVariance[eOps] = pstChnPass->stChnPassPerfStatics.u32CurAccumVariance[eOps]/pstChnPass->stChnPassPerfStatics.u32CurAccumFPS[eOps];
            pstChnPass->stChnPassPerfStatics.u32LastUpdatedAverage[eOps] = pstChnPass->stChnPassPerfStatics.u32CurAccumTimeInNS[eOps]/pstChnPass->stChnPassPerfStatics.u32CurAccumFPS[eOps];
        }
        else
        {
            pstChnPass->stChnPassPerfStatics.u32LastUpdatedVariance[eOps] = 0;
            pstChnPass->stChnPassPerfStatics.u32LastUpdatedAverage[eOps] = 0;
        }
        pstChnPass->stChnPassPerfStatics.u32LastUpdatedMaxVal[eOps] = pstChnPass->stChnPassPerfStatics.u32AccumMaxVal[eOps];
        pstChnPass->stChnPassPerfStatics.u32AccumMaxVal[eOps] = 0;
        pstChnPass->stChnPassPerfStatics.u32CurAccumVariance[eOps] = 0;
        pstChnPass->stChnPassPerfStatics.u32CurAccumFPS[eOps] = 0;
        pstChnPass->stChnPassPerfStatics.u32CurAccumTimeInNS[eOps] = 0;
        pstChnPass->stChnPassPerfStatics.u32LastUpdateFPSJiffies[eOps] = jiffies;
    }
}

static inline void _MI_SYS_IMPL_DevChnPassPerf(MI_SYS_DevPass_t *pstDevPass, MI_SYS_DEVPASS_OPS_TYPE eOps, MI_U32 u32TimeCostInNs)
{
    MI_SYS_BUG_ON(eOps>=MI_SYS_DEVOPS_MAX);
    MI_SYS_BUG_ON(!pstDevPass || pstDevPass->u32MagicNumber != __MI_SYS_DEVPASS_MAGIC_NUM__);
    pstDevPass->stDevPassPerfStatics.u32CurAccumFPS[eOps]++;
    pstDevPass->stDevPassPerfStatics.u32CurAccumVariance[eOps] += MI_SYS_ABC((MI_S32)u32TimeCostInNs-(MI_S32)pstDevPass->stDevPassPerfStatics.u32LastUpdatedAverage[eOps]);
    pstDevPass->stDevPassPerfStatics.u32AccumMaxVal[eOps] = MI_SYS_MAX(pstDevPass->stDevPassPerfStatics.u32AccumMaxVal[eOps], u32TimeCostInNs);
    pstDevPass->stDevPassPerfStatics.u32CurAccumTimeInNS[eOps] += u32TimeCostInNs;

    if(_mi_time_diff(pstDevPass->stDevPassPerfStatics.u32LastUpdateFPSJiffies[eOps], jiffies)>=HZ)
    {
        pstDevPass->stDevPassPerfStatics.u32LastUpdatedFPS[eOps] = pstDevPass->stDevPassPerfStatics.u32CurAccumFPS[eOps];
        if(pstDevPass->stDevPassPerfStatics.u32CurAccumFPS[eOps])
        {
            pstDevPass->stDevPassPerfStatics.u32LastUpdatedVariance[eOps] = pstDevPass->stDevPassPerfStatics.u32CurAccumVariance[eOps]/pstDevPass->stDevPassPerfStatics.u32CurAccumFPS[eOps];
            pstDevPass->stDevPassPerfStatics.u32LastUpdatedAverage[eOps] = pstDevPass->stDevPassPerfStatics.u32CurAccumTimeInNS[eOps]/pstDevPass->stDevPassPerfStatics.u32CurAccumFPS[eOps];
        }
        else
        {
            pstDevPass->stDevPassPerfStatics.u32LastUpdatedVariance[eOps] = 0;
            pstDevPass->stDevPassPerfStatics.u32LastUpdatedAverage[eOps] = 0;
        }
        pstDevPass->stDevPassPerfStatics.u32LastUpdatedMaxVal[eOps] = pstDevPass->stDevPassPerfStatics.u32AccumMaxVal[eOps];
        pstDevPass->stDevPassPerfStatics.u32AccumMaxVal[eOps] = 0;
        pstDevPass->stDevPassPerfStatics.u32CurAccumVariance[eOps] = 0;
        pstDevPass->stDevPassPerfStatics.u32CurAccumFPS[eOps] = 0;
        pstDevPass->stDevPassPerfStatics.u32CurAccumTimeInNS[eOps] = 0;
        pstDevPass->stDevPassPerfStatics.u32LastUpdateFPSJiffies[eOps] = jiffies;
    }
}

static inline const char *_MI_SYS_IMPL_ChnPass_ActionName(MI_SYS_CHNPASS_OPS_TYPE eOps)
{
    switch(eOps)
    {
        case MI_SYS_CHNOPS_PREPROCESS_INPUTTASK:
            return "OnPreProcessInputTask";
        case MI_SYS_CHNOPS_ENQUEUE_INPUTTASK:
            return "EnqueueInputTask";
        case MI_SYS_CHNOPS_BARRIER_INPUTTASK:
            return "BarrierInputTask";
        case MI_SYS_CHNOPS_CHECK_INPUTTASK:
            return "CheckInputTaskStatus";
        case MI_SYS_CHNOPS_DEQUEUE_INPUTTASK:
            return "DequeueInputTask";
        case MI_SYS_CHNOPS_POLL_ASYNC_OUTPUTTASK:
            return "OnPollingAsyncOutputTaskConfig";
        case MI_SYS_CHNOPS_ENQUEUE_ASYNC_OUTPUTTASK:
            return "EnqueueAsyncOutputTask";
        case MI_SYS_CHNOPS_BARRIER_ASYNC_OUTPUTTASK:
            return "BarrierAsyncOutputTask";
        case MI_SYS_CHNOPS_CHECK_OUTPUTTASK:
            return "CheckOutputTaskStatus";
        case MI_SYS_CHNOPS_DEQUEUE_OUTPUTTASK:
            return "DequeueOutputTask";
        default:
            MI_SYS_BUG();
            return "INVALID_CHNPASS_OPS";
    }
}
static inline const char *_MI_SYS_IMPL_DEVPass_ActionName(MI_SYS_DEVPASS_OPS_TYPE eOps)
{
    switch(eOps)
    {
        case MI_SYS_DEVOPS_ASYNC_PULSE_KICKOFF_DELAY:
            return "PulseBufTransferDelay";
        case MI_SYS_DEVOPS_KICKOFF_INTERVAL:
            return "KickoffInterval";
        case MI_SYS_DEVOPS_NEXTQUEUE_PENDING_DELAY:
            return "NextQueuePendingTime";
        default:
            MI_SYS_BUG();
            return "INVALID_DEVPASS_OPS";
    }
}

/* if timeout 200MS, warnning on consle */
#define EXEC_TIMEOUT_WARN_SECONDS   (200)
#define EXEC_WITH_TIMEOUT_CHECK(expr, ops, timeout_in_ms, pstChnPass)  \
    do{   \
        MI_U64 u64BegTickInNs = mi_sys_get_boot_time_in_ns(); \
        MI_U32 u32DiffNs; \
        expr; \
        u32DiffNs = (MI_U32)(mi_sys_get_boot_time_in_ns()-u64BegTickInNs); \
        _MI_SYS_IMPL_StaticChnPassPerf(pstChnPass, ops, u32DiffNs); \
        if(u32DiffNs >=(EXEC_TIMEOUT_WARN_SECONDS * 1000000UL)) \
            DBG_WRN("%s costs %dus[M%d:D%d:P%d]\n", _MI_SYS_IMPL_ChnPass_ActionName(ops), u32DiffNs/1000UL, \
                (pstChnPass)->pstDevPass->pstModDev->eModuleId, (pstChnPass)->pstDevPass->pstModDev->u32DevId,(pstChnPass)->pstDevPass->s32PassId); \
        else if(u32DiffNs >=((timeout_in_ms) * 1000000UL)) \
            DBG_KMSG("%s costs %dus[M%d:D%d:P%d]\n", _MI_SYS_IMPL_ChnPass_ActionName(ops), u32DiffNs/1000UL, \
                (pstChnPass)->pstDevPass->pstModDev->eModuleId, (pstChnPass)->pstDevPass->pstModDev->u32DevId,(pstChnPass)->pstDevPass->s32PassId); \
   }while(0)

#endif
