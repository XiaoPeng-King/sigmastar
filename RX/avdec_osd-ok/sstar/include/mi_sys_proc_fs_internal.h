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
#ifndef _MI_SYS_PROC_FS_INTERNEL_H_
#define _MI_SYS_PROC_FS_INTERNEL_H_
#include "mi_common_internal.h"

typedef enum {
    E_MI_COMMON_POLL_NOT_READY         = (0x0)     ,
    E_MI_COMMON_FRAME_READY_FOR_READ   = (0x1 << 0),
    E_MI_COMMON_BUFFER_READY_FOR_WRITE = (0x1 << 1),
} MI_COMMON_PollFlag_e;

typedef struct MI_COMMON_PollFile_s {
    void *private;
} MI_COMMON_PollFile_t;

#define MI_COMMON_GetPoll(__ppstPollFile, __fState, __fRelease) do{ \
    MI_S32 MI_COMMON_CreatePoll(MI_COMMON_PollFile_t **, MI_COMMON_PollFlag_e (*)(MI_COMMON_PollFile_t*), void (*)(MI_COMMON_PollFile_t*)); \
    MI_COMMON_CreatePoll(__ppstPollFile, __fState, __fRelease); \
}while(0)

#define MI_COMMON_ReleasePollFd(__ppstPollFile) do{ \
    MI_S32 MI_COMMON_PollRelease(MI_COMMON_PollFile_t **); \
    MI_COMMON_PollRelease(__ppstPollFile); \
}while(0)

MI_S32 MI_COMMON_WakeUpClient(MI_COMMON_PollFile_t *pstClient);
//struct kobject *MI_DEVICE_GetDebugObject(void);

#define MI_COMMON_AddDebugFile(__name, __write, __read) do{ \
    MI_S32 MI_COMMON_SetupDebugFile(const char *, MI_S32 (*)(MI_SYS_DEBUG_HANDLE_t ,const char **, MI_S32), MI_S32 (*)(MI_SYS_DEBUG_HANDLE_t)); \
    MI_COMMON_SetupDebugFile(__name, __write, __read); \
}while(0)

#define MI_COMMON_AddDebugRawFile(__name, __write, __read) do{ \
    MI_S32 MI_COMMON_SetupDebugRawFile(const char *,  MI_S32 (*)(const char *, MI_U64, MI_S32), MI_S32 (*)(char *, MI_U64, MI_S32)); \
    MI_COMMON_SetupDebugRawFile(__name, __write, __read); \
}while(0)

#if defined(__linux__)
#define MI_COMMON_GetSelfDir ({ \
    extern struct proc_dir_entry *MI_DEVICE_GetSelfDir(void); \
    struct proc_dir_entry *MI_COMMON_RealGetSelfDir(struct proc_dir_entry *);\
    MI_COMMON_RealGetSelfDir(MI_DEVICE_GetSelfDir());\
})
#else
#define MI_COMMON_GetSelfDir (NULL)
#endif
#endif /* _MI_SYS_PROC_FS_INTERNEL_H_ */

