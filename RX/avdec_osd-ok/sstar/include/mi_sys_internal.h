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
#ifndef _MI_SYS_INTERNAL_H_
#define _MI_SYS_INTERNAL_H_

#include "mi_common.h"
#include "mi_sys_datatype.h"
#include "mhal_common.h"
#include "mhal_cmdq.h"
#include "mi_common_internal.h"
#include "mi_print.h"
#include "cam_os_wrapper.h"
#include "cam_os_util.h"
#include "cam_os_util_list.h"
#if defined(__linux__)
#include "ms_platform.h"
#endif


#define MI_SYS_PROC_FS_DEBUG
#define MI_DISP_PROCFS_DEBUG (1)
#define MI_VIF_PROCFS_DEBUG (1)
#define MI_VDISP_PROCFS_DEBUG (1)
#define MI_UAC_PROCFS_DEBUG (1)
#define MI_DIVP_PROCFS_DEBUG (1)
#define MI_VPE_PROCFS_DEBUG (1)
#define MI_AI_PROCFS_DEBUG (1)
#define MI_AO_PROCFS_DEBUG (1)
#define MI_HDMI_PROCFS_DEBUG (1)
#define MI_GFX_PROCFS_DEBUG (1)
#define MI_RGN_PROCFS_DEBUG (1)
#define MI_VDEC_PROCFS_DEBUG (1)
#define MI_WARP_PROCFS_DEBUG (1)
#define MI_LDC_PROCFS_DEBUG (1)
#define MI_PANEL_PROCFS_DEBUG (1)
#define MI_CIPHER_PROCFS_DEBUG (1)
#define MI_SHADOW_PROCFS_DEBUG (1)
#define MI_IPU_PROCFS_DEBUG (1)
//#define MI_SYS_SERIOUS_ERR_MAY_MULTI_TIMES_SHOW
#define MI_SYS_MAX_DEV_PASS_CNT 4

typedef void *  MI_SYS_DRV_HANDLE;
typedef void * MI_SYS_DEV_PASS_HANDLE;

#define ALIGN_DOWN(val, alignment) (((val)/(alignment))*(alignment))
#define ALIGN_UP(val, alignment) ((( (val)+(alignment)-1)/(alignment))*(alignment))

static inline unsigned long _mi_time_diff(unsigned long start, unsigned long end)
{
#ifdef CAM_OS_RTK
   if(end<start)
       return   (unsigned long)(0xFFFFFFFF/6000 - ( long)start + (long)end);
   else
       return   (unsigned long)(( long)end - (long)start);

#else
    return   (unsigned long)(( long)end - (long)start);
#endif
}

#define _MI_SYS_WaitEventTimeout(Waitqueue,   condition,    s32TimeOutMs)  \
do{  \
    MI_U32 u32BegTick = jiffies;  \
    MI_U32 u32CurTick;   \
while(1)  \
{  \
    if(condition)  \
        break;  \
   CamOsTcondTimedWait(&(Waitqueue),(s32TimeOutMs));  \
   u32CurTick =jiffies;   \
   if(_mi_time_diff(u32BegTick,u32CurTick)>msecs_to_jiffies(s32TimeOutMs))  \
      break;  \
}  \
}while(0)

#define WAKE_UP_QUEUE_IF_NECESSARY(waitqueue)  \
    do{   \
            CamOsTcondSignalAll(&(waitqueue)); \
    }while(0)

#define MI_SYS_BUG_ON(exp)    \
    do{    \
        if(exp)    \
        {    \
            MI_PRINT("[%s][%d]Case %s BUG ON!!!\n", __PRETTY_FUNCTION__, __LINE__, #exp);    \
            BUG();\
        }    \
    }while(0)

#define MI_SYS_BUG() MI_SYS_BUG_ON(1)

#define MI_MMU_MMA_HEAP "MMU_MMA"

#define MI_SYS_CHIP_NAME_I5      "I5"
#define MI_SYS_CHIP_NAME_I6      "I6"
#define MI_SYS_CHIP_NAME_I2M     "I2M"
#define MI_SYS_CHIP_NAME_I6E     "I6E"
#define MI_SYS_CHIP_NAME_I6B0    "I6b0"

typedef enum
{
    E_MI_SYS_CHIP_IDX_I5,
    E_MI_SYS_CHIP_IDX_I6,
    E_MI_SYS_CHIP_IDX_I2M,
    E_MI_SYS_CHIP_IDX_I6E,
    E_MI_SYS_CHIP_IDX_I6B0,
    E_MI_SYS_CHIP_IDX_MAX
} mi_sys_ChipIdx_e;

typedef struct mi_sys_ChipInfo_s
{
    mi_sys_ChipIdx_e eChipIdx;
    char *szChipName;
}mi_sys_ChipInfo_t;

extern MI_U32 SSTAR_CONFIG_HZ;
typedef enum
{
    // driver accept current task and continue iterator
    MI_SYS_ITERATOR_ACCEPT_CONTINUTE,
    // driver reject current task and continue iterator,
    MI_SYS_ITERATOR_SKIP_CONTINUTE,
    // driver accept current task and ask to stop iterator
    MI_SYS_ITERATOR_ACCEPT_STOP,
} mi_sys_TaskIteratorCBAction_e;

#ifdef MI_SYS_PROC_FS_DEBUG

typedef struct MI_SYS_DEBUG_HANDLE_s
{
    MI_S32 (*OnPrintOut)( struct MI_SYS_DEBUG_HANDLE_s  handle, char *data,...);
    void *file;
} MI_SYS_DEBUG_HANDLE_t;

typedef struct mi_sys_ModuleDevProcfsOps_s
{
    //these callbacks include all dump info that needed!!!
    //if these callbacks  success,please return MI_SUCCESS,do not return MI_DEF_ERR(E_MI_SYS_MODULE_ID_xx, E_MI_ERR_LEVEL_yy, MI_SUCCESS)
    MI_S32 (*OnDumpDevAttr)(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId,void *pUsrData);
    MI_S32 (*OnDumpChannelAttr)(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData);
    MI_S32 (*OnDumpInputPortAttr)(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData);
    MI_S32 (*OnDumpOutPortAttr)(MI_SYS_DEBUG_HANDLE_t  handle, MI_U32  u32DevId, void *pUsrData);
    MI_S32 (*OnHelp)(MI_SYS_DEBUG_HANDLE_t  handle,MI_U32  u32DevId,void *pUsrData);
} mi_sys_ModuleDevProcfsOps_t;

typedef struct allocator_echo_cmd_info_s
{
    MI_U8 dir_name[256];
    MI_U32 dir_size;
    MI_U32 offset;
    MI_U32 length;
} allocator_echo_cmd_info_t;

typedef struct mi_sys_AllocatorProcfsOps_s
{
    MI_S32 (*OnDumpAllocatorAttr)(MI_SYS_DEBUG_HANDLE_t  handle,void *private);
    MI_S32 (*OnAllocatorExecCmd)(MI_SYS_DEBUG_HANDLE_t  handle,allocator_echo_cmd_info_t *cmd_info,void *private);
} mi_sys_AllocatorProcfsOps_t;
#endif

typedef struct mi_sys_ChnTaskInfo_s
{
    struct CamOsListHead_t listChnTask;//for driver facility
    MI_U32 u32Reserved0;
    MI_U32 u32Reserved1;
    MI_U32 u32Reserved2;
    MI_U32 u32Reserved3;
    MI_U64 u64Reserved0;//for driver facility
    MI_U64 u64Reserved1;

    MI_U64 u64Reserved2;
    MI_U64 u64Reserved3;
    MI_U64 u64Reserved4;
    MI_U64 u64Reserved5;
    MI_U64 u64Reserved6;

    MI_SYS_DRV_HANDLE miSysDrvHandle;
    ///which channel
    MI_U32 u32ChnId;
    ///buffer will lock and remove pending list
    MI_SYS_BufInfo_t *pastInputPortBufInfo[MI_SYS_MAX_INPUT_PORT_CNT];

    //Fill in the buf configuration information by the moudle owner
    MI_SYS_BufConf_t   astOutputPortPerfBufConfig[MI_SYS_MAX_OUTPUT_PORT_CNT];

    ///buffer will lock and remove free buffer list
    MI_SYS_BufInfo_t *pastOutputPortBufInfo[MI_SYS_MAX_OUTPUT_PORT_CNT];
    ///frame rate control
    MI_BOOL bOutputPortMaskedByFrmrateCtrl[MI_SYS_MAX_OUTPUT_PORT_CNT];
    unsigned char padding[8];
} mi_sys_ChnTaskInfo_t;


typedef struct mi_sys_ChnInputTaskInfo_s
{
    ///which channel
    MI_U32 u32ChnId;
    MI_U32 u32MaxValidInputPortNum;
    //for driver saving private tacking data during period between EnqueueInputTask()&DequeueInputTask();
    void *pInputTaskDrvPrivData;
    ///buffer will lock and remove pending list
    MI_SYS_BufInfo_t *pastInputPortBufInfo[MI_SYS_MAX_INPUT_PORT_CNT];
    MI_U16 u16TaskFence;
} mi_sys_ChnInputTaskInfo_t;

typedef struct mi_sys_ChnTaskOutBufConfig_s
{
    MI_U32 u32ChnId;
    ///buffer will lock and remove pending list
    MI_BOOL   bNeedAllocOutputBuf[MI_SYS_MAX_OUTPUT_PORT_CNT];
    MI_BOOL   bSupportInplaceProcess[MI_SYS_MAX_OUTPUT_PORT_CNT];
    MI_SYS_BufConf_t   astOutputPortPerfBufConfig[MI_SYS_MAX_OUTPUT_PORT_CNT];
} mi_sys_ChnTaskOutBufConfig_t;

typedef struct mi_sys_ChnOutputTaskInfo_s
{
    ///which channel
    MI_U32 u32ChnId;
    ///buffer will lock and remove pending list
    MI_U32 u32MaxValidOutputPortNum;
    ///for driver saving private tacking data during period between EnqueueAsyncOutputTask()/EnqueueInputTask&DequeueOutputTask();
    void *pOutputTaskDrvPrivData;
    MI_BOOL bOutputPortMaskedByFrmrateCtrl[MI_SYS_MAX_OUTPUT_PORT_CNT];
    MI_SYS_BufInfo_t *pastOutputPortBufInfo[MI_SYS_MAX_OUTPUT_PORT_CNT];
    MI_U16 u16TaskFence;
    MI_BOOL bRewindOutputPortBuf[MI_SYS_MAX_OUTPUT_PORT_CNT];
} mi_sys_ChnOutputTaskInfo_t;

typedef struct mi_sys_ChnBufInfo_s
{
    //channel number
    MI_U32 u32ChannelId;

    MI_U32 u32InputPortNum;
    ///input port pending buffer from peer binded channel
    MI_U16 au16InputPortBindConnectBufPendingCnt[MI_SYS_MAX_INPUT_PORT_CNT];
    ///input port pending buffer from user insert
    MI_U16 au16InputPortUserBufPendingCnt[MI_SYS_MAX_INPUT_PORT_CNT];

    ///input port haved locked count
    MI_U16 au16InputPortBufHoldByDrv[MI_SYS_MAX_INPUT_PORT_CNT];


    MI_U32 u32OutputPortNum;
    /// output buffer  count hold by user through MI_SYS_Get_Outputport_Buf();
    MI_U16 au16OutputPortBufUsrLockedNum[MI_SYS_MAX_OUTPUT_PORT_CNT];
    /// output buffer  count in user fifo list
    MI_U16 au16OutputPortBufInUsrFIFONum[MI_SYS_MAX_OUTPUT_PORT_CNT];
    //output buffer current hold by MI Driver
    MI_U16 au16OutputPortBufHoldByDrv[MI_SYS_MAX_OUTPUT_PORT_CNT];

    /// output total in used buffer count
    MI_U16 au16OutputPortBufTotalInUsedNum[MI_SYS_MAX_OUTPUT_PORT_CNT];
} mi_sys_ChnBufInfo_t;

struct MI_SYS_BufferAllocation_s;
typedef struct buf_allocation_ops_s
{
    void (*OnRef)(struct MI_SYS_BufferAllocation_s *thiz);
    void (*OnUnref)(struct MI_SYS_BufferAllocation_s *thiz);
    void (*OnRelease)(struct MI_SYS_BufferAllocation_s *thiz);
    void *(*map_user)(struct MI_SYS_BufferAllocation_s *thiz);
    void  (*unmap_user)(struct MI_SYS_BufferAllocation_s *thiz);
    void *(*vmap_kern)(struct MI_SYS_BufferAllocation_s *thiz);
    void (*vunmap_kern)(struct MI_SYS_BufferAllocation_s *thiz);
    void (*shrink_content)(struct MI_SYS_BufferAllocation_s *thiz);
} buf_allocation_ops_t;

typedef void (*MI_SYS_Allocation_Free_Notify)(struct MI_SYS_BufferAllocation_s *allocation, void *pUsrdata);

typedef struct MI_SYS_BufferAllocation_s
{
    unsigned int u32MagicNumber;
    CamOsAtomic_t  ref_cnt;
    buf_allocation_ops_t ops;
    MI_SYS_BufInfo_t stBufInfo;
    void *private1;
    void *private2;
    bool bMmuAddr;
} MI_SYS_BufferAllocation_t;

struct mi_sys_Allocator_s;
typedef struct buf_allocator_ops_s
{
    MI_S32 (*OnRef)(struct mi_sys_Allocator_s *thiz);
    MI_S32 (*OnUnref)(struct mi_sys_Allocator_s *thiz);
    MI_S32 (*OnRelease)(struct mi_sys_Allocator_s *thiz);
    MI_SYS_BufferAllocation_t *(*alloc)(struct mi_sys_Allocator_s *pstAllocator, const char *buf_name, MI_SYS_BufConf_t *pstBufConfig);
    int (*suit_bufconfig)(struct mi_sys_Allocator_s *pstAllocator, MI_SYS_BufConf_t *pstBufConfig);
} buf_allocator_ops_t;

typedef struct mi_sys_Allocator_s
{
    CamOsAtomic_t  ref_cnt;
    unsigned int u32MagicNumber;
    struct CamOsListHead_t list;
    buf_allocator_ops_t *ops;
    void *pPrivate;
} mi_sys_Allocator_t;

typedef enum
{
    E_MI_SYS_PASS_SKIP_NONE,
    E_MI_SYS_PASS_SKIP_TEMPORARY,
    E_MI_SYS_PASS_SKIP_PERSISTENT
} mi_sys_PassSkipMode_e;

typedef enum
{
    E_MI_SYS_TASK_RUNING,
    E_MI_SYS_TASK_DONE,
    E_MI_SYS_TASK_RETRY_LATER,
    E_MI_SYS_TASK_DROP
} mi_sys_TaskStatus_e;
typedef enum
{
    E_MI_SYS_TASK_PREPARE_SUCCESS,
    E_MI_SYS_TASK_PREPARE_BUSY,
    E_MI_SYS_TASK_PREPARE_DROP
} mi_sys_TaskPrepareRes_e;


typedef struct mi_sys_PortBindCaps_s
{
    MI_U32 u32PitchAlignment;
    MI_U32 u32HPixelAlignment;
    MI_U32 u32VPixelAlignment;
    MI_U32 u32ExtSupportedBindMode;
    MI_U32 u32RingModeLineCntExpMask;//base 2 exponential mask, meanful in case BIND_TYPE_HW_SYNC_RING_MODE ON
} mi_sys_PortBindCaps_t;

typedef struct mi_sys_BindSetting_s
{
    MI_U32 u32PitchAlignment;
    MI_U32 u32HPixelAlignment;
    MI_U32 u32VPixelAlignment;
    MI_U32 u32BindMode;
    MI_U32 u32RingLineCnt;//ring buffer line count
} mi_sys_BindSetting_t;
typedef struct mi_sys_BindChnPort_s
{
    MI_ModuleId_e eModId;
    MI_U32  u32DevId;
    MI_U32 u32ChnId;
    MI_U32 u32PassId;
    MI_U32 u32PortId;
} mi_sys_BindChnPort_t;

typedef struct mi_sys_DevPassOpsInfo_s
{
    //related ops
    mi_sys_TaskPrepareRes_e (*OnPreProcessInputTask)(struct MHAL_CMDQ_CmdqInterface_s *pstMHALCMDQ, mi_sys_ChnInputTaskInfo_t *pstInputTask, mi_sys_ChnTaskOutBufConfig_t *pstBufConfig, void *pUsrData);
    mi_sys_TaskPrepareRes_e (*OnPrepareOutputTaskMetaInfo)(struct MHAL_CMDQ_CmdqInterface_s *pstMHALCMDQ, mi_sys_ChnInputTaskInfo_t *pstInputTask, mi_sys_ChnOutputTaskInfo_t *pstOutputTask, void *pUsrData);
    mi_sys_TaskStatus_e (*EnqueueInputTask)(struct MHAL_CMDQ_CmdqInterface_s *pstMHALCMDQ, mi_sys_ChnInputTaskInfo_t *pstInputTask, mi_sys_ChnOutputTaskInfo_t *pstOutputTask, void *pUsrData);
    MI_S32 (*BarrierInputTask)(struct MHAL_CMDQ_CmdqInterface_s *pstMHALCMDQ, MI_U32 u32ChannelId, void *pUsrData);
    mi_sys_TaskPrepareRes_e (*OnPrepareAsyncOutputTaskMetaInfo)(struct MHAL_CMDQ_CmdqInterface_s *pstMHALCMDQ, mi_sys_ChnOutputTaskInfo_t *pstOutputTask, void *pUsrData);
    mi_sys_TaskStatus_e (*EnqueueAsyncOutputTask)(struct MHAL_CMDQ_CmdqInterface_s *pstMHALCMDQ, mi_sys_ChnOutputTaskInfo_t *pstOutputTask, void *pUsrData);
    MI_S32 (*BarrierAsyncOutputTask)(struct MHAL_CMDQ_CmdqInterface_s *pstMHALCMDQ, MI_U32 u32ChannelId, void *pUsrData);
    mi_sys_TaskStatus_e (*CheckInputTaskStatus)(struct MHAL_CMDQ_CmdqInterface_s *pstMHALCMDQ, mi_sys_ChnInputTaskInfo_t *pstInputTask, MI_U32 u32MillsecAfterTaskEnqueued, void *pUsrData);
    mi_sys_TaskStatus_e (*DequeueInputTask)(struct MHAL_CMDQ_CmdqInterface_s *pstMHALCMDQ, mi_sys_ChnInputTaskInfo_t *pstInputTask, void *pUsrData);
    mi_sys_TaskStatus_e (*CheckOutputTaskStatus)(struct MHAL_CMDQ_CmdqInterface_s *pstMHALCMDQ, mi_sys_ChnOutputTaskInfo_t *pstOutputTask, MI_U32 u32MillsecAfterTaskEnqueuedvoid, void *pUsrData);
    mi_sys_TaskStatus_e (*DequeueOutputTask)(struct MHAL_CMDQ_CmdqInterface_s *pstMHALCMDQ, mi_sys_ChnOutputTaskInfo_t *pstOutputTask, void *pUsrData);
    MI_S32 (*OnPollingAsyncOutputTaskConfig)(struct MHAL_CMDQ_CmdqInterface_s *pstMHALCMDQ, mi_sys_ChnTaskOutBufConfig_t *pstOutBufConfig, MI_U32 u32TimeoutMs, void *pUsrData);
    MI_BOOL (*NeedIndividualWorkThread)(MI_SYS_DRV_HANDLE hDrvHandle, MI_U32 *pu32DummySyncRegAddr, void *pUsrData);

    //bind & unbind ops
    mi_sys_PassSkipMode_e (*CheckPassSkipMode)(MI_SYS_DRV_HANDLE hDrvHandle, MI_U32 u32ChannelId, void *pUsrData);
    MI_S32 (*OnGetInputPortBindCapability)(MI_SYS_DRV_HANDLE hDrvHandle, mi_sys_BindChnPort_t *pstChnCurInputPort, mi_sys_BindChnPort_t *pstChnPeerOutputPort,
                                           mi_sys_PortBindCaps_t *pstPortBindCaps, void *pUsrData);
    MI_S32 (*OnGetOutputPortBindCapability)(MI_SYS_DRV_HANDLE hDrvHandle, mi_sys_BindChnPort_t *pstChnCurOutputPort, mi_sys_BindChnPort_t *pstChnPeerInputPort,
                                            mi_sys_PortBindCaps_t *pstPortBindCaps, void *pUsrData);
    MI_S32 (*OnBindInputPort)(MI_SYS_DRV_HANDLE hDrvHandle, mi_sys_BindChnPort_t *pstChnCurInputPort, mi_sys_BindChnPort_t *pstChnPeerOutputPort,
                              mi_sys_BindSetting_t *pstBindSetting, void *pUsrData);
    MI_S32 (*OnUnBindInputPort)(MI_SYS_DRV_HANDLE hDrvHandle, mi_sys_BindChnPort_t *pstChnCurInputPort, mi_sys_BindChnPort_t *pstChnPeerOutputPort, void *pUsrData);
    MI_S32 (*OnBindOutputPort)(MI_SYS_DRV_HANDLE hDrvHandle, mi_sys_BindChnPort_t *pstChnCurOutputPort, mi_sys_BindChnPort_t *pstChnPeerInputPort,
                               mi_sys_BindSetting_t *pstBindSetting, void *pUsrData);
    MI_S32 (*OnUnBindOutputPort)(MI_SYS_DRV_HANDLE hDrvHandle, mi_sys_BindChnPort_t *pstChnCurOutputPort, mi_sys_BindChnPort_t *pstChnPeerInputPort, void *pUsrData);
    MI_S32 (*OnOutputPortBufRelease)(MI_SYS_DRV_HANDLE hDrvHandle, mi_sys_BindChnPort_t *pstChnCurryPort, MI_SYS_BufInfo_t *pstBufInfo);
    //Get the port id when inner bind inside this module channel
    MI_U32 (*OnGetPassInputPortNum)(void);
    MI_U32 (*OnGetPassOutputPortNum)(void);
    MI_U32 (*OnGetPassInnerBindInputPort)(MI_SYS_DRV_HANDLE hDrvHandle, MI_U32 u32ChnId, MI_U32 u32PeerOutputPassId, void *pUsrData);
    MI_U32 (*OnGetPassInnerBindOutputPort)(MI_SYS_DRV_HANDLE hDrvHandle, MI_U32 u32ChnId, MI_U32 u32PeerInputPassId, void *pUsrData);

    MI_BOOL (*OnSupportOutputTaskReorder)(void);

    int self_inernal_barrier;
    int reject_cmdq;
    int force_accept_multi_inputtsk_count;
    MI_U16 u16InputTaskTimeoutMS;
    MI_U16 u16OutputTaskTimeoutMS;
    MI_U32 u32CmdqBufSize;
    //void *private_data;
} mi_sys_DevPassOpsInfo_t;

typedef struct mi_sys_ModuleDevOps_s
{
    MI_S32 (*onGetInterBindInputPortPassId)(MI_SYS_DRV_HANDLE hDrvHandle, MI_SYS_ChnPort_t *pstChnPort, MI_U32 *u32PassId , MI_U32 *u32PortId ,void *pUsrData);
    MI_S32 (*onGetInterBindOutputPortPassId)(MI_SYS_DRV_HANDLE hDrvHandle, MI_SYS_ChnPort_t *pstChnPort, MI_U32 *u32PassId , MI_U32 *u32PortId, void *pUsrData);
} mi_sys_ModuleDevOps_t;

typedef struct mi_sys_ModuleDevInfo_s
{
    MI_ModuleId_e eModuleId;
    MI_U32 u32DevId;
    MI_U32 u32DevInputPortNum;//the valid input port num of the first pass, the other pass's input port num is one
    MI_U32 u32DevOutputPortNum;//the valid output port num of the last pass, the other pass's output port num is one
    MI_U32 u32DevChnNum;
    MI_U32 u32DevPassNum;
    MI_BOOL bEnableMmu;
    MI_U32 u32ThreadPriority;
    MI_BOOL bEnableChnIsrCheck;
    mi_sys_ModuleDevOps_t astModuleDevOps;
    mi_sys_DevPassOpsInfo_t astDevPassOps[MI_SYS_MAX_DEV_PASS_CNT];
} mi_sys_ModuleDevInfo_t;

//get boot time in nanoseconds
static inline MI_S64 mi_sys_get_boot_time_in_ns(void)
{
    CamOsTimespec_t ts;
    MI_U64 u64_current_boot_ns;

    //get_monotonic_boottime(&ts);
    CamOsGetMonotonicTime(&ts);
    u64_current_boot_ns = ts.nSec*1000000000ULL+ts.nNanoSec;

    return (MI_S64)u64_current_boot_ns;
}

typedef void * MI_SYS_USER_MAP_CTX;

#ifdef MI_SYS_PROC_FS_DEBUG
MI_S32 mi_sys_allocator_proc_create(MI_U8 *allocator_proc_name,mi_sys_AllocatorProcfsOps_t *Ops,void *allocator_private_data);
void mi_sys_allocator_proc_remove_allocator(MI_U8 *entry_name);
#endif

void generic_allocation_on_ref(struct MI_SYS_BufferAllocation_s *pstBufAllocation);

void generic_allocation_on_unref(struct MI_SYS_BufferAllocation_s *pstBufAllocation);

//use this function to replace mi_sys_buf_mgr_init_std_allocation function
void generic_allocation_init(MI_SYS_BufferAllocation_t *pstBufAllocation,buf_allocation_ops_t *ops);

MI_S32 generic_allocator_on_ref(struct mi_sys_Allocator_s *pstAllocator);

MI_S32 generic_allocator_on_unref(struct mi_sys_Allocator_s *pstAllocator);

void generic_allocator_init(mi_sys_Allocator_t *pstAllocator, buf_allocator_ops_t *ops);

MI_SYS_DRV_HANDLE  mi_sys_RegisterDev(mi_sys_ModuleDevInfo_t *pstModuleInfo, void *pUsrData
#ifdef MI_SYS_PROC_FS_DEBUG
                                      , mi_sys_ModuleDevProcfsOps_t *pstModuleProcfsOps
                                      , struct proc_dir_entry *dir_entry
#endif
                                     );

void mi_sys_NotifyWakeupISREvent(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_S32 s32PassId);

void mi_sys_NotifyWakeupISREventByChn(MI_SYS_DRV_HANDLE miSysDrvHandle,MI_S32 s32PassId, MI_S32 s32ChnId);

MI_S32 mi_sys_UnRegisterDev(MI_SYS_DRV_HANDLE miSysDrvHandle);

#ifdef MI_SYS_PROC_FS_DEBUG
MI_S32 mi_sys_RegistCommand(MI_U8 *u8Cmd, MI_U8 u8MaxPara,
                            MI_S32 (*fpExecCmd)(MI_SYS_DEBUG_HANDLE_t handle, MI_U32 u32DevId, MI_U8 argc, MI_U8 **argv, void *pUsrData),
                            MI_SYS_DRV_HANDLE hHandle);
#endif

MI_S32 mi_sys_GetChnBufInfo(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PassId, mi_sys_ChnBufInfo_t *pstChnBufInfo);

typedef mi_sys_TaskIteratorCBAction_e (* mi_sys_TaskIteratorCallBack)(mi_sys_ChnTaskInfo_t *pstTaskInfo, void *pUsrData);

MI_S32 mi_sys_EnableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId);

MI_S32 mi_sys_DisableChannel(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId);

MI_S32 mi_sys_EnableOutputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId, MI_U32 u32PortId);

MI_S32 mi_sys_DisableOutputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId, MI_U32 u32PortId);

MI_S32 mi_sys_SetOutputPortBufExtConf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_S32 s32PassId, MI_U32 u32PortId, MI_SYS_FrameBufExtraConfig_t *pstBufExtraConf);

MI_S32 mi_sys_SetOutputPortBufClearConf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_S32 s32PassId,MI_U32 u32PortId, MI_BOOL bClear);

MI_S32 mi_sys_SetOutputPortUserFIFOBlockMode(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_U32 u32PortId, MI_BOOL mode);

MI_S32 mi_sys_SetInputPortBufExtConf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_S32 s32PassId,MI_U32 u32PortId, MI_SYS_FrameBufExtraConfig_t *pstBufExtraConf);

MI_S32 mi_sys_SetInputPortBufClearConf(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId , MI_S32 s32PassId,MI_U32 u32PortId,  MI_BOOL bClear);

MI_S32 mi_sys_EnableInputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId ,MI_S32 s32PassId, MI_U32 u32PortId);

MI_S32 mi_sys_DisableInputPort(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId, MI_U32 u32PortId);

MI_S32 mi_sys_SetInputPortSidebandMsg(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId, MI_U32 u32PortId, MI_U64 u64SidebandMsg);

MI_S32 mi_sys_SetReg (MI_U32 u32RegAddr, MI_U16 u16Value, MI_U16 u16Mask);

MI_S32 mi_sys_GetReg (MI_U32 u32RegAddr, MI_U16 *pu16Value);

/*
N.B.
below MMAHeapName can only be NULL or real mma heap name, do not set it with random character string.
you can get mma heap name xxx from "mma_heap=xxx," of cat /proc/cmdline.
*/

MI_S32 mi_sys_MMA_Alloc(MI_U8 *u8MMAHeapName, MI_U8 *bufName, MI_U32 u32blkSize ,MI_PHY *phyAddr);

MI_S32 mi_sys_MMA_Alloc_Timeout(MI_U8 *u8MMAHeapName, MI_U8 *u8BufName, MI_U32 u32blkSize ,MI_PHY *phyAddr, MI_U32 u32TimeMs);

MI_S32 mi_sys_MMA_Free(MI_PHY phyAddr);

void * mi_sys_Vmap(MI_PHY u64PhyAddr, MI_U32 u32Size, MI_BOOL bCache);

void mi_sys_UnVmap(void *pVirtAddr);

MI_S32 mi_sys_VFlushInvCache(void *pVirtAddr, MI_U32 u32Size);

MI_S32 mi_sys_EnsureOutportBkRefFifoDepth (MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId, MI_U32 u32PortId, MI_U32 u32BkRefFifoDepth);

MI_S32 mi_sys_MmapBufToMultiUserVirAddr(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_SYS_BufInfo_t *pstBufInfo, void **ppBufHandle, MI_BOOL bCache);

MI_S32 mi_sys_UnmapBufToMultiUserVirAddr(MI_SYS_DRV_HANDLE miSysDrvHandle, void *pBufHandle, MI_SYS_BufInfo_t **ppstBufInfo, MI_BOOL bFlushCache);

MI_PHY mi_sys_Cpu2Miu_BusAddr(MI_PHY cpu_addr);

MI_PHY mi_sys_Miu2Cpu_BusAddr(MI_PHY miu_phy_addr);

//the following are APIs to support MI modules' customer allocator implement
MI_S32 mi_sys_SetInputPortUsrAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId, MI_U32 u32PortId, struct mi_sys_Allocator_s *pstUserAllocator);

MI_S32 mi_sys_SetOutputPortUsrAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32PortId, mi_sys_Allocator_t *pstUserAllocator);
mi_sys_Allocator_t *mi_sys_SysmemAllocatorGet(void);

MI_S32 mi_sys_SetPeerOutputPortCusAllocator(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_S32 s32PassId, MI_U32 u32PortId, struct mi_sys_Allocator_s *pstUserAllocator);

MI_SYS_BufferAllocation_t* mi_sys_AllocMMALowLevelBufAllocation(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_SYS_BufConf_t *pstBufConfig);

MI_S32 mi_sys_NotifyPreProcessBuf(MI_SYS_BufInfo_t *buf);

MI_SYS_BufInfo_t* mi_sys_InnerAllocBufFromVbPool(MI_SYS_DRV_HANDLE miSysDrvHandle, MI_U32 u32ChnId, MI_U32 u32blkSize, MI_PHY *phyAddr);

MI_S32 mi_sys_FreeInnerVbPool(MI_SYS_BufInfo_t* pBufAllocation);

MI_S32 mi_sys_ConfDevPubPools(MI_ModuleId_e  eModule, MI_U32 u32DevId, MI_VB_PoolListConf_t  stPoolListConf);

MI_S32 mi_sys_RelDevPubPools(MI_ModuleId_e  eModule, MI_U32 u32DevId);

MI_U32 mi_sys_Get_Vdec_VBPool_UseStatus(void);
MI_S32 mi_sys_DropTask(mi_sys_ChnTaskInfo_t *pstTask);
MI_U32 mi_sys_Gfx_Ops_Callback(void *ops);

#ifdef __KERNEL__
#if LINUX_VERSION_CODE == KERNEL_VERSION(4,9,84)
MI_S32 mi_sys_MemsetPa(MI_PHY pa, MI_U32 u32Val, MI_U32 u32Lenth);
MI_S32 mi_sys_MemcpyPa(MI_PHY paDst, MI_PHY paSrc, MI_U32 u32Lenth);
MI_S32 mi_sys_BufFillPa(MI_SYS_FrameData_t *pstBuf, MI_U32 u32Val, MI_SYS_WindowRect_t *pstRect);
MI_S32 mi_sys_BufBlitPa(MI_SYS_FrameData_t *pstDstBuf, MI_SYS_WindowRect_t *pstDstRect, MI_SYS_FrameData_t *pstSrcBuf, MI_SYS_WindowRect_t *pstSrcRect);
#endif
#endif

MI_S32 mi_sys_DropTask(mi_sys_ChnTaskInfo_t *pstTask);

MI_S32 mi_sys_RingPool_Create(MI_ModuleId_e eModule , MI_U32 u32Devid , MI_S32 s32ChnId , MI_U8 *strMMAHeapName, MI_U8 *strRingPoolName,
        MI_SYS_RingPool_Config_t *pstRingPoolConfig, MI_SYS_RINGPOOL_HANDLE *pHandle);

MI_S32 mi_sys_RingPool_Destroy(MI_ModuleId_e eModule , MI_U32 u32Devid , MI_S32 s32ChnId,MI_SYS_RINGPOOL_HANDLE handle);

MI_S32 mi_sys_RingPool_EnsureTailSpaceInISR(MI_SYS_RINGPOOL_HANDLE handle,
        MI_SYS_RingPool_Record_t *pNextFreeSpace, MI_BOOL bPosRest);

MI_S32 mi_sys_RingPool_EnsureTailSpaceTimeout(MI_SYS_RINGPOOL_HANDLE handle,
        MI_U32 u32MaxMsToWait, MI_SYS_RingPool_Record_t *pNextFreeSpace, MI_BOOL bPosRest);

MI_S32 mi_sys_Ringpool_GetHeadRecordInISR(MI_SYS_RINGPOOL_HANDLE handle,
        MI_SYS_RingPool_Record_t *pNextContentRecord, MI_BOOL bReadFlag);

MI_S32 mi_sys_RingPool_GetHeadRecordTimeout(MI_SYS_RINGPOOL_HANDLE handle,
        MI_SYS_RingPool_Record_t *pNextContentRecord, MI_BOOL bReadFlag, MI_U32 u32MaxMsToWait);

MI_S32 mi_sys_RingPool_ReleaseHeadRecord(MI_SYS_RINGPOOL_HANDLE handle, MI_BOOL bFromISR);

MI_S32 mi_sys_Ringpool_GetRecordTotalNum(MI_SYS_RINGPOOL_HANDLE handle, MI_BOOL bFromISR, MI_U32 *pu32Num);

MI_S32 mi_sys_Ringpool_GetRecordUnreadNum(MI_SYS_RINGPOOL_HANDLE handle, MI_BOOL bFromISR, MI_U32 *pu32Num);

MI_S32 mi_sys_Ringpool_GetHeadRecordTotalNum(MI_SYS_RINGPOOL_HANDLE handle, MI_BOOL bFromISR, MI_U32 *pu32Num);

MI_S32 mi_sys_RingPool_AppendTailContentTailRecord(MI_SYS_RINGPOOL_HANDLE handle, MI_BOOL bFromISR, MI_BOOL bContentClosed,
        MI_SYS_RingPool_Record_t *pNextContentRecord);

MI_S32 mi_sys_RingPool_CancelUnFinishedTailRecord(MI_SYS_RINGPOOL_HANDLE handle, MI_BOOL bFromISR);

MI_S32 mi_sys_RingPool_ReleaseTailRecord(MI_SYS_RINGPOOL_HANDLE handle, MI_BOOL bFromISR);

MI_S32 mi_sys_RingPool_GetRingPoolTotalSpace(MI_SYS_RINGPOOL_HANDLE handle, MI_SYS_RingPool_Record_t *pNextFreeSpace);

MI_S32 mi_sys_RingPool_DumpRecordInfo(MI_SYS_RINGPOOL_HANDLE handle);

MI_S32 mi_sys_GetCurPts(MI_U64 *pu64Pts);

MI_S32 mi_sys_GetChnMMAConf(MI_ModuleId_e eModId,  MI_U32  u32DevId, MI_U32 u32ChnId, void  *pu8MMAHeapName, MI_U32 u32Length);

MI_S32 mi_sys_PrivateDevChnHeapAlloc(MI_ModuleId_e eModule , MI_U32 u32Devid , MI_S32 s32ChnId, MI_U8 *u8BufName, MI_U32 u32blkSize ,MI_PHY *phyAddr ,MI_BOOL bTailAlloc);

MI_S32 mi_sys_PrivateDevChnHeapFree(MI_ModuleId_e eModule , MI_U32 u32Devid , MI_S32 s32ChnId, MI_PHY phyAddr);

MI_SYS_USER_MAP_CTX mi_sys_GetCur_UserMapCtx(void);

MI_S32 mi_sys_IsMmuMma(MI_BOOL *pbMmuMma);

MI_S32 mi_sys_Mmap(MI_PHY phyAddr, MI_U32 u32Size, void **ppVirtualAddress, MI_BOOL bCaChe);

MI_S32 mi_sys_Munmap(void *pVirtualAddress, MI_U32 u32Size);

MI_S32 mi_sys_SetChnOutputPortDepth(MI_SYS_ChnPort_t *pstChnPort, MI_U32 u32UserFrameDepth, MI_U32 u32BufQueueDepth);

MI_S32 mi_sys_ChnOutputPortPutBuf(MI_SYS_BUF_HANDLE BufHandle);

MI_S32 mi_sys_Read_Uuid(MI_U64 *u64Uuid);

static inline char * mi_sys_GetPlatformName(void)
{
    char *szChipName = NULL;
#if defined(__linux__)
    szChipName = Chip_Get_Platform_Name();
#else
    #if (defined CONFIG_SIGMASTAR_CHIP_I6) && (CONFIG_SIGMASTAR_CHIP_I6 == 1)
        szChipName = MI_SYS_CHIP_NAME_I6;
    #elif (defined CONFIG_SIGMASTAR_CHIP_I5) && (CONFIG_SIGMASTAR_CHIP_I5 == 1)
        szChipName = MI_SYS_CHIP_NAME_I5;
    #elif (defined CONFIG_SIGMASTAR_CHIP_I2M) && (CONFIG_SIGMASTAR_CHIP_I2M == 1)
        szChipName = MI_SYS_CHIP_NAME_I2M;
    #elif (defined CONFIG_SIGMASTAR_CHIP_I6E) && (CONFIG_SIGMASTAR_CHIP_I6E == 1)
        szChipName = MI_SYS_CHIP_NAME_I6E;
    #elif (defined CONFIG_SIGMASTAR_CHIP_I6B0) && (CONFIG_SIGMASTAR_CHIP_I6B0 == 1)
        szChipName = MI_SYS_CHIP_NAME_I6B0;
    #else
        szChipName = "NONE";
    #endif
#endif

    return szChipName;
}

static inline mi_sys_ChipIdx_e mi_sys_GetPlatFormIdx(void)
{
    char *szChipName = NULL;
    mi_sys_ChipIdx_e eIdx = E_MI_SYS_CHIP_IDX_MAX;
    mi_sys_ChipInfo_t astChipInfo[] =
    {
        {.eChipIdx = E_MI_SYS_CHIP_IDX_I5, .szChipName = MI_SYS_CHIP_NAME_I5},
        {.eChipIdx = E_MI_SYS_CHIP_IDX_I6, .szChipName = MI_SYS_CHIP_NAME_I6},
        {.eChipIdx = E_MI_SYS_CHIP_IDX_I2M, .szChipName = MI_SYS_CHIP_NAME_I2M},
        {.eChipIdx = E_MI_SYS_CHIP_IDX_I6E, .szChipName = MI_SYS_CHIP_NAME_I6E},
        {.eChipIdx = E_MI_SYS_CHIP_IDX_I6B0, .szChipName = MI_SYS_CHIP_NAME_I6B0}
    };

    szChipName = mi_sys_GetPlatformName();
    for (eIdx=E_MI_SYS_CHIP_IDX_I5; eIdx<E_MI_SYS_CHIP_IDX_MAX && eIdx<sizeof(astChipInfo)/sizeof(mi_sys_ChipInfo_t); ++eIdx)
    {
        if (!strncasecmp(astChipInfo[eIdx].szChipName, szChipName,  strlen(astChipInfo[eIdx].szChipName) > strlen(szChipName) ? strlen(astChipInfo[eIdx].szChipName) : strlen(szChipName)))
            break;
    }

    if (eIdx == E_MI_SYS_CHIP_IDX_MAX)
        DBG_ERR("can't find valid platform id\n");

    return eIdx;
}
///////////////////////////////////////////////
// crash system if product hash verified fail
///////////////////////////////////////////////
void mi_sys_ProductHashGuard(void);
#ifdef  __KERNEL__
#define DEBUG_YUV_KERN_API
#endif
#ifdef DEBUG_YUV_KERN_API
typedef  struct file* FILE_HANDLE;

//return value of the following API is not standard file operation return value.

/*
return :
FILE_HANDLE type in fact is a pointer,
NULL:fail
not NULL:success
*/
FILE_HANDLE open_yuv_file(const char *pathname, int open_for_write);

/*
return value:
0:success
-1:fail
*/
int read_yuv_file(FILE_HANDLE filehandle, MI_SYS_FrameData_t framedata);

/*
return value:
0:success
-1:fail
*/
int write_yuv_file(FILE_HANDLE filehandle, MI_SYS_FrameData_t framedata);

void close_yuv_file(FILE_HANDLE filehandle);

/*
return value:
0:success
-1:fail
*/
int reset_yuv_file(FILE_HANDLE filehandle);

/*
return value:
0:success
-1:fail
*/
int is_in_yuv_file_end(FILE_HANDLE filehandle);
#endif
#if 0
#define _MI_SYS_DEBUG_CLEAR_VAR_PTR(var_ptr)  \
    do{  \
        int cur_size;  \
        char *local_ptr = (char*)(var_ptr);  \
        cur_size = sizeof(*(var_ptr));  \
        if(cur_size>=4)  \
        {  \
            *(int*)local_ptr = __LINE__;  \
            cur_size -=4;  \
            local_ptr +=4;  \
        }  \
        if(cur_size > 0)  \
        {  \
           strncpy(local_ptr, __FUNCTION__, cur_size);  \
        }  \
    }while(0)
#else
#define _MI_SYS_DEBUG_CLEAR_VAR_PTR(var_ptr)  \
    do{  \
        int cur_size;  \
        unsigned long *local_ptr = (unsigned long*)(var_ptr);  \
        cur_size = sizeof(*(var_ptr));  \
        while(cur_size>= sizeof(unsigned long))  \
        {  \
            *local_ptr = __LINE__;  \
            cur_size -=sizeof(unsigned long);  \
            local_ptr++;  \
        }  \
    }while(0)

#endif
#endif ///_MI_SYS_INTERNAL_H_
