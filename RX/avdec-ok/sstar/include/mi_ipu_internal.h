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

#ifndef _MI_IPU_INTERNAL_H_
#define _MI_IPU_INTERNAL_H_

#include "cam_os_wrapper.h"
#include "cam_fs_wrapper.h"
#include "mi_ipu.h"

#define PAGESIZE    (4096)
#ifdef ALIGN_UP
#undef ALIGN_UP
#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
#else
#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
#endif

typedef struct MI_IPU_TensorDescInternal_s
{
   int u32TensorDim;
   int s32AlignedBufSize;
   MI_IPU_ELEMENT_FORMAT eElmFormat;
   MI_BOOL bFP32Out;
   float stScalar;
   MI_U32 u32TensorShape[MI_IPU_MAX_TENSOR_DIM];
   MI_S8 name[MAX_TENSOR_NAME_LEN];
} MI_IPU_TensorDescInternal_t;

typedef struct MI_IPU_SubNet_InputOutputDescInternal_s
{
   MI_U32 u32InputTensorCount;
   MI_U32 u32OutputTensorCount;
   MI_IPU_TensorDescInternal_t stMI_InputTensorShapes[MI_IPU_MAX_INPUT_TENSOR_CNT];
   MI_IPU_TensorDescInternal_t stMI_OutputTensorShapes[MI_IPU_MAX_OUTPUT_TENSOR_CNT];
} MI_IPU_SubNet_InputOutputDescInternal_t;

typedef struct MI_IPU_TensorInternal_s
{
    //MI_BOOL bShapeAvailable;
    //MI_BOOL bRewiseRGBOrder;//only valid under U8 & C=3,4 or NV12
    //MI_IPU_TensorShape_t stTensorShape;
    MI_PHY  phyTensorAddr[2];//notice that this is miu bus addr,not cpu bus addr.
} MI_IPU_TensorInternal_t;

typedef struct MI_IPU_TensorVectorInternal_s
{
    MI_U32 u32TensorCount;
    MI_IPU_TensorInternal_t  stArrayTensors[MI_IPU_MAX_TENSOR_CNT];
} MI_IPU_TensorVectorInternal_t;

#ifdef PERFORMANCE_DEBUG
#if defined(__USER__)
#include <sys/time.h>
#elif defined(__KERNEL__)
#include <linux/time.h>
#endif
typedef struct IPU_Perf_Stat_s {
    unsigned int firmware_statistic[8];
    struct timeval kernel_tv[8];
    struct timeval user_tv[8];
} IPU_Perf_Stat_t;
#endif

typedef struct MI_IPU_InOutTensorVector_s {
    MI_U32 u32Chn;
    MI_IPU_TensorVectorInternal_t inputTensorVentor;
    MI_IPU_TensorVectorInternal_t outputTensorVentor;
    MI_BOOL bForceOnlineMode;
#ifdef PERFORMANCE_DEBUG
    IPU_Perf_Stat_t perf_stat;
#endif
} MI_IPU_InOutTensorVector_t;

typedef struct MI_IPU_SingleGroup_s {
    MI_PHY phyAddr[MI_IPU_MAX_TENSOR_CNT];
    void *virAddr[MI_IPU_MAX_TENSOR_CNT];
    MI_BOOL bAvailable;
} MI_IPU_SingleGroup_t;

typedef struct MI_IPU_PrivatePool_s {
    MI_IPU_SingleGroup_t singleGroup[MAX_IPU_INPUT_OUTPUT_BUF_DEPTH];
} MI_IPU_PrivatePool_t;

typedef struct MI_IPU_CHN_s {
    MI_U32 u32ChnId;
    MI_U32 modelFileSize;
    MI_PHY phyAddrModel;
    void *pVirtualAddress;
    MI_PHY phyAddrDesc;
    MI_IPU_SubNet_InputOutputDescInternal_t *chnInOutDesc;
    MI_IPU_PrivatePool_t privateInputPool, privateOutputPool;
    MI_U32 u32InputBufDepth, u32OutputBufDepth;
    CamOsMutex_t inputBufferLock, outputBufferLock;
    MI_U32 privatePoolSize;
} MI_IPU_CHN_t;

typedef enum {
    CREATE_CHANNEL = 0,
    DESTROY_CHANNEL,
    INVOKE,
    SET_MALLOC_REGION,
    SET_IPU_PARAMETER,
    GET_IPU_STATISTIC,
} IPU_Operation;

typedef enum {
    RPC_TOKEN_HANDSHAKE,
    RPC_TOKEN_CMD,
} RPC_TOKEN;

typedef struct IPU_info_s {
    int count;
    unsigned int info[8];
} IPU_info_t;

// IPU data type
#define SGS_MAGIC "SGSI"
#define SGS_MAGIC_SIZE 4
#define SGS_MAX_INPUTS 6
#define SGS_MAX_OUTPUTS 8
#define SGS_MAX_DIMS 8

typedef enum {
    E_TF_TENSOR_TYPE_FLOAT32 = 0,
    E_TF_TENSOR_TYPE_FLOAT16,
    E_TF_TENSOR_TYPE_INT32,
    E_TF_TENSOR_TYPE_UINT8,
    E_TF_TENSOR_TYPE_INT64,
    E_TF_TENSOR_TYPE_STRING,
    E_TF_TENSOR_TYPE_BOOL,
    E_TF_TENSOR_TYPE_INT16,
    E_TF_TENSOR_TYPE_COMPLEX64,
    E_TF_TENSOR_TYPE_INT8,
    E_TF_TENSOR_TYPE_MAX,
} SGS_TF_TensorType_e;

typedef struct {
    MI_S32                  s32Tensorshape[SGS_MAX_DIMS];
    MI_S32                  s32Stride;
    SGS_TF_TensorType_e       eTensorType;
} SGS_Tensor_t;

typedef enum {
    SGS_ENCRYPTION_MODE_UNKNOWN = -1,
    SGS_ENCRYPTION_MODE_CTR = 0,
    SGS_ENCRYPTION_MODE_CBC,
    SGS_ENCRYPTION_MODE_ECB,
} SGS_ENCRYPTION_MODE_e;

typedef enum {
    SGS_HASH_UNKNOWN = -1,
    SGS_HASH_SHA1 = 0,
    SGS_HASH_SHA256,
} SGS_HASH_ALG_e;

struct sgs_img_hdr {
    MI_U8 u8Magic[SGS_MAGIC_SIZE];
    SGS_Tensor_t stInputs[SGS_MAX_INPUTS];
    MI_S32 s32InputNum;
    SGS_Tensor_t stOutputs[SGS_MAX_OUTPUTS];
    MI_S32 s32OutputNum;

    MI_U32 u32ModelSize;  /* size in bytes */
    MI_U32 u32BufferSize;  /* virable tensor buffer size in bytes */
    MI_U32 u32OfflineCmdSize; /* size in bytes */
    SGS_HASH_ALG_e eHashAlg;   /* sha1 / sha256 */
    MI_U32 u32Hash[8]; /* 256-bits sha1 / sha256 */
    MI_U8 u8EncryptionAlg; /*AES ...etc*/
    SGS_ENCRYPTION_MODE_e eMode;
    MI_U8 u8IV[16];/*128-bits  iv */
    MI_U8 u8PaddingType; /* block's padding values*/
    MI_U32 u32PageSize;    /* flash page size we assume */
} __attribute__((packed));

typedef void (*neon_vector)(short*, float*, float*);

typedef struct vector_struct_s {
    int vector_size;
    int vector_cnt;
    neon_vector neon_func;
} vector_struct_t;

typedef struct MI_IPU_InitFirmware_s {
    MI_PHY phyFirmware;
    MI_PHY firmware_heap_start;
    MI_PHY firmware_heap_end;
    MI_PHY firmware_code_addr;
    MI_PHY firmware_data_addr;
    MI_U32 firmware_code_size;
    MI_U32 firmware_data_size;
} MI_IPU_InitFirmware_t;

#define IPU_MAX_MMA_NAME_LEN    (32)
#define IPU_DEVICE_CHN4POOL     (-1)
#define IPU_FRIMWARE_MMA_NAME       "ipu_firmware"
#define IPU_VARIABLE_MMA_NAME       "ipu_variable"
#define IPU_HEAP_MMA_NAME       "ipu_heap"

#endif /* _MI_IPU_INTERNAL_H_ */
