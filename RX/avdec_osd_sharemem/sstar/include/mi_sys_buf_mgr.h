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
#ifndef _MI_SYS_BUFMGR_H_
#define _MI_SYS_BUFMGR_H_

#include "cam_os_wrapper.h"

#include "mi_sys_buf_mgr.h"
#include "mi_common.h"
#include "mi_sys_datatype.h"
#include "mi_common_datatype.h"
#include "mi_sys_datatype.h"
#include "mi_sys_internal.h"
#include "mi_scatterlist.h"



#define __MI_SYS_ALLOCATOR_MAGIC_NUM__ 0x4D414C43
#define __MI_SYS_COLLECTION_MAGIC_NUM__ 0x4D434F4C
#define __MI_SYS_BUF_REF_MAGIC_NUM__ 0x42554652

#define MI_SYS_META_DATA_SIZE_MAX 256

#define SKIP_CHECK_MASK     (1<<0)
#define SKIP_AUTOFREE_MASK  (1<<1)
#define ALIGN_UP(val, alignment) ((( (val)+(alignment)-1)/(alignment))*(alignment))
/*******************************
LOW_LATENCY_ENABLE_CALC_TIME

0: not calc sw lowlatency time
1: calc sw lowlatency time on sw lowlatency mode enable
2: calc sw lowlatency time on sw lowlatency mode disable

********************************/
#define LOW_LATENCY_ENABLE_CALC_TIME 0

/*******************************
LOW_LATENCY_ENABLE_LIST_TIME

0: framemode none lowlatency
1: framemode lowlatency
2: realtime mode none lowlatency
3: realtime mode lowlatency

********************************/
#define LOW_LATENCY_ENABLE_LIST_TIME 0

struct MI_SYS_BufRef_s;

typedef void (*OnBufRefRelFunc)(struct MI_SYS_BufRef_s *pstBufRef, void *pCBData);

typedef struct MI_SYS_BufRef_s
{
    unsigned int u32MagicNumber;
    struct CamOsListHead_t list;
    struct MI_SYS_BufferAllocation_s *pstBufAllocation;
    OnBufRefRelFunc onRelCB;
    void *pCBData;
    MI_SYS_BufInfo_t bufinfo;
    MI_BOOL is_dumped;
    MI_U32 u32ExtFlag;
    struct CamOsListHead_t list_in_pass;//list head in dev pass ordered by time stampe
    struct CamOsListHead_t list_in_port;//list head in port in FIFO way
    MI_U32 u32ChannelId;
    union
    {
        MI_S64 s64FireTimeStampInNS;// the fire time stamp, MI_SYS_FIRE_IMMEDIATELY for fire immediately
    } uTimeStamp;
    MI_U32 u32InferFrameNumber;
    MI_U32 u32EnqueueInferPulseIndex;//the infer pulse index when buffer eqnueued
    MI_S64 s64BufContentTimeStampInNS; // buffer generated time stamp for performance measurement

    struct MI_SYS_BufRef_s *pstSrcBufRef;//pointer to source buffer reference
} MI_SYS_BufRef_t;
#if (LOW_LATENCY_ENABLE_CALC_TIME == 1 || LOW_LATENCY_ENABLE_CALC_TIME == 2)
typedef struct MI_SYS_BufTrackStaticInfo_s
{
    MI_U64 u64AccumTotalDelayInUS;
    MI_U64 u64AccumTotalLastDelayInUS;
    MI_U32 u32AccumCount;
} MI_SYS_BufTrackStaticInfo_t;
#endif
typedef enum
{
    MI_SYS_QUEUE_ID_USR_INJECT,
    MI_SYS_QUEUE_ID_USR_GET_FIFO,
    MI_SYS_QUEUE_ID_BIND_INPUT,
    MI_SYS_QUEUE_ID_DRIVER_BACK_REF,
    MI_SYS_QUEUE_ID_WORKING,
} MI_SYS_QueueId_e;

typedef struct MI_SYS_BufferQueue_s
{
    CamOsMutex_t mtx;
    struct CamOsListHead_t list;
    int  queue_buf_count;
    MI_BOOL dump_buffer_wq_cond;//condition of wait queue for dump buffer
    CamOsTcond_t dump_buffer_wq;
    MI_ModuleId_e module_id;
    MI_SYS_QueueId_e  queue_id; /* 0~5 */
    unsigned char  device_id; /* 0~3 */
    unsigned char  channel_id;  /* 0~63 */
    unsigned char  port;  /* 0~15 */
    unsigned char  io;  /* 0~1 */
    unsigned char  pass_id; /* 0~3 */
#if (LOW_LATENCY_ENABLE_CALC_TIME == 1 || LOW_LATENCY_ENABLE_CALC_TIME == 2)
    MI_SYS_BufTrackStaticInfo_t stEnqueueBufObjectTrackStaticInfo;
    MI_SYS_BufTrackStaticInfo_t stDequeueBufObjectTrackStaticInfo;
#endif
} MI_SYS_BufferQueue_t;

typedef struct MI_SYS_AllocatorRef_s
{
    struct CamOsListHead_t list;
    struct mi_sys_Allocator_s *pstAllocator;
} MI_SYS_AllocatorRef_t;

typedef struct MI_SYS_Allocator_Collection_s
{
    unsigned int u32MagicNumber;
    CamOsTsem_t semlock;
    struct CamOsListHead_t list;
    int  collection_size;
} MI_SYS_Allocator_Collection_t;

typedef  struct  MI_SYS_FrameSpecial_s
{
    MI_U32 phy_offset[3];
    MI_U32 u32Stride[3];
    MI_U32 u32BufSize;//total size that allocated for this buffer,include consider alignment.

} MI_SYS_FrameSpecial_t;


typedef struct mi_sys_mmap_data_s
{
    unsigned long pfn;
    unsigned long pages;
    MI_BOOL bCaChe;
} mi_sys_mmap_data_t;

#if (LOW_LATENCY_ENABLE_CALC_TIME == 1 || LOW_LATENCY_ENABLE_CALC_TIME == 2)
static inline void mi_sys_buf_mgr_accum_trackinfo(MI_SYS_BufTrackStaticInfo_t  *pstBufTrackInfo,
        MI_SYS_BufRef_t *pstBufRef)
{
    struct CamOsTimespec_t tv;

    CamOsGetTimeOfDay(&tv);
    if(pstBufRef == NULL)
        return;
    if(!pstBufRef->is_traceInfoValid)
        return;
    pstBufTrackInfo->u64AccumTotalDelayInUS = tv.nSec*1000000 + tv.nNanoSec/1000 - pstBufRef->u32ObjectInitTimeInUS;
    pstBufTrackInfo->u64AccumTotalLastDelayInUS = tv.nSec*1000000 + tv.nNanoSec/1000 - pstBufRef->u32ObjectAccessedTimeInUS;
    pstBufTrackInfo->u32AccumCount++;
    if(pstBufTrackInfo->u32AccumCount>0x100)
    {
        pstBufTrackInfo->u32AccumCount >>=1;
        pstBufTrackInfo->u64AccumTotalDelayInUS >>=1;
        pstBufTrackInfo->u64AccumTotalLastDelayInUS >>=1;
    }
}
#if 0
static inline void mi_sys_buf_mgr_read_trackinfo(mi_sys_latency_static_t *pstStaticInfo,
        MI_U32 *pu32PipeLineAverageDelay, MI_U32 *pu32LastStageAverageDelay)  //not use current
{
    if(pstStaticInfo->u32AccumCount==0)
    {
        *pu32PipeLineAverageDelay = 0;
        *pu32LastStageAverageDelay = 0;
    }
    else
    {
        *pu32PipeLineAverageDelay = pstStaticInfo->u64PipeLineAccumUS / pstStaticInfo->u32AccumCount;
        *pu32LastStageAverageDelay += pstStaticInfo->u64WorkQueueLatencyAccumUS/pstStaticInfo->u32AccumCount;
    }
}
#endif
static inline void mi_sys_buf_mgr_sync_trackinfo(MI_SYS_BufInfo_t *pstInputBuf, MI_SYS_BufInfo_t *pstOutputBuf) //not use current
{
    MI_SYS_BufRef_t *pstInputBufRef = container_of(pstInputBuf, MI_SYS_BufRef_t, bufinfo);
    MI_SYS_BufRef_t *pstOutputBufRef = container_of(pstOutputBuf, MI_SYS_BufRef_t, bufinfo);

    BUG_ON(!pstInputBufRef || !pstOutputBufRef);
    BUG_ON(pstOutputBufRef->is_traceInfoValid);

    pstOutputBufRef->u32ObjectInitTimeInUS = pstInputBufRef->u32ObjectInitTimeInUS;
    pstOutputBufRef->u32ObjectAccessedTimeInUS = pstInputBufRef->u32ObjectAccessedTimeInUS;
    pstOutputBufRef->is_traceInfoValid = pstInputBufRef->is_traceInfoValid;
}
static inline void mi_sys_buf_mgr_update_trackinfo(MI_SYS_BufRef_t *pstBufRef)
{
    struct timeval tv;

    do_gettimeofday(&tv);
    if(pstBufRef)
        pstBufRef->u32ObjectAccessedTimeInUS = tv.tv_sec*1000000 + tv.tv_usec;//-u32TouchedUSBefore; default set 0
}


static inline void mi_sys_buf_mgr_init_trackinfo(MI_SYS_BufInfo_t *pstBuf, MI_U32 u32TouchedUSBefore)
{
    struct timeval tv;
    MI_SYS_BufRef_t *pstBufRef = container_of(pstBuf, MI_SYS_BufRef_t, bufinfo);
    BUG_ON(!pstBufRef);
    //should be less than 1S before
    BUG_ON(u32TouchedUSBefore>1000*1000);
    do_gettimeofday(&tv);
    pstBufRef->u32ObjectInitTimeInUS = tv.tv_sec*1000000 + tv.tv_usec;//init time set 0
    pstBufRef->u32ObjectAccessedTimeInUS = tv.tv_sec*1000000 + tv.tv_usec;// - u32TouchedUSBefore; default set 0
    pstBufRef->is_traceInfoValid = TRUE;
}
#endif


MI_S32 mi_sys_attach_allocator_to_collection(mi_sys_Allocator_t *pstAllocator, MI_SYS_Allocator_Collection_t *pstAllocatorCollection);
MI_S32 mi_sys_detach_allocator_from_collection(mi_sys_Allocator_t *pstAllocator, MI_SYS_Allocator_Collection_t *pstAllocatorCollection);
void mi_sys_init_allocator_collection(MI_SYS_Allocator_Collection_t *pstAllocatorCollection);
void mi_sys_deinit_allocator_collection(MI_SYS_Allocator_Collection_t *pstAllocatorCollection);
MI_SYS_BufferAllocation_t * mi_sys_alloc_from_allocator_collection(MI_SYS_Allocator_Collection_t *pstAllocatorCollection,  MI_SYS_BufConf_t *pstBufConfig);
unsigned long mi_sys_buf_mgr_get_size(MI_SYS_BufConf_t *pstBufConfig, MI_SYS_FrameSpecial_t  *pstFrameDataInfo);

MI_SYS_USER_MAP_CTX mi_sys_buf_get_cur_user_map_ctx(void);
int mi_sys_buf_mgr_user_map(struct sg_table *sg_table,void **pp_user_map_ptr, void *key, unsigned int ops_flag);
int mi_sys_buf_mgr_user_unmap(void *key);
void  mi_sys_buf_mgr_user_unmap_by_process(void);

MI_S32 mi_sys_buf_mgr_flush_inv_cache(void *va,unsigned long len);

struct MI_SYS_BufRef_s *mi_sys_create_bufref( MI_SYS_BufferAllocation_t *pstBufAllocation,
        MI_SYS_BufConf_t *pstBufConfig, OnBufRefRelFunc onRelCB, void *pCBData, MI_U32 u32ChnId);
struct MI_SYS_BufRef_s *mi_sys_create_realtime_bufref(MI_SYS_BufConf_t *pstBufConfig, OnBufRefRelFunc onRelCB, void *pCBData, MI_U32 u32ChnId);

struct MI_SYS_BufRef_s *mi_sys_dup_bufref(MI_SYS_BufRef_t *pstBufRef, OnBufRefRelFunc onRelCB, void *pCBData, MI_U32 u32ChnId);

void mi_sys_release_bufref(struct MI_SYS_BufRef_s *pstBufRef);

void mi_sys_init_buf_queue(struct MI_SYS_BufferQueue_s *pstQueue, MI_ModuleId_e eModuleId, MI_SYS_QueueId_e eQueueId, unsigned char device_id, unsigned char channel_id, unsigned char pass_id, unsigned char port, unsigned char io);
void mi_sys_add_to_queue_head(struct  MI_SYS_BufRef_s *pstBufRef, struct MI_SYS_BufferQueue_s*pstQueue);
void mi_sys_add_to_queue_tail( struct MI_SYS_BufRef_s *pstBufRef, struct MI_SYS_BufferQueue_s*pstQueue);
struct MI_SYS_BufRef_s* mi_sys_remove_from_queue_head(struct MI_SYS_BufferQueue_s*pstQueue);
struct MI_SYS_BufRef_s* mi_sys_remove_from_queue_tail(struct MI_SYS_BufferQueue_s*pstQueue);
void mi_sys_remove_from_queue(MI_SYS_BufRef_t *pstBufRef, MI_SYS_BufferQueue_t*pstQueue);
#ifdef MI_SYS_PROC_FS_DEBUG
MI_U64 mi_sys_calc_buf_size_in_queue(MI_SYS_BufferQueue_t*pstQueue);
#endif
MI_S32 mi_sys_buf_map_kern_va(MI_SYS_BufferAllocation_t*pstBufAllocation, MI_SYS_BufInfo_t *pstBufInfo, MI_BOOL bflashflag);

int mi_sys_buf_mgr_vmap_kern(struct sg_table *sg_table,void **pp_kern_map_ptr, unsigned int ops_flag);
int mi_sys_buf_mgr_vunmap_kern(void *va_in_kern);
int mi_sys_check_page_range_mapable(unsigned long pfn, long count);
mi_sys_Allocator_t *mi_sys_get_global_meta_allocator(void);
void mi_sys_buf_mgr_fill_bufinfo(MI_SYS_BufInfo_t *pstBufInfo,
                                 MI_SYS_BufConf_t *pstBufConfig, MI_SYS_FrameSpecial_t *pstFrameDataInfo, MI_PHY  phyaddr);
MI_U32 _mi_clear_framebuf_pading_area(MI_SYS_FrameData_t *pstFrameData, MI_SYS_BufFrameConfig_t *pstFrameCfg);

#endif
