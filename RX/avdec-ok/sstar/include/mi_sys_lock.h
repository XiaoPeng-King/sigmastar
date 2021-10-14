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
#ifndef _MI_SYS_LOCK_H_
#define _MI_SYS_LOCK_H_

//#define CONFIG_MI_SYS_ENABLE_DETECT_DEADLOCK
#if (defined __KERNEL__) && (defined CONFIG_MI_SYS_ENABLE_DETECT_DEADLOCK)
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/seq_file.h>
#include "cam_os_wrapper.h"

#ifndef TASK_COMM_LEN
#define TASK_COMM_LEN 16
#endif

#define MI_SYS_LOCK_NAME_LENGTH 64

typedef struct MI_SYS_LockNode_s
{
    struct CamOsListHead_t list_node_lock;
    char szName[MI_SYS_LOCK_NAME_LENGTH]; //lock name
    char szFunc[MI_SYS_LOCK_NAME_LENGTH]; //caller function name
    int line; //caller function line
}MI_SYS_LockNode_t;

typedef struct MI_SYS_TaskLock_s
{
    struct CamOsListHead_t list_node_task_lock;
    struct CamOsListHead_t list_head_lock;
    CamOsTsem_t list_head_lock_semlock;
    pid_t pid; //task pid
    char szName[TASK_COMM_LEN]; //task name
    MI_SYS_LockNode_t *pstRequestLockNode; //the lock which the task is requesting.
}MI_SYS_TaskLock_t;

MI_SYS_TaskLock_t* _MI_SYS_FindTaskLockInstance(void);
MI_SYS_LockNode_t* _MI_SYS_FindLockNodeByName(MI_SYS_TaskLock_t *pstTaskLock, char *szLockName);
MI_SYS_LockNode_t* _MI_SYS_CreateNewLockNode(char *szLockName);
void _MI_SYS_ReleaseLockNode(MI_SYS_TaskLock_t *pstTaskLock, MI_SYS_LockNode_t *pstLockNode);
void _MI_SYS_AttachLockNodeToList(MI_SYS_LockNode_t *pstLockNode, MI_SYS_TaskLock_t *pstTaskLock);
void _MI_SYS_CheckLock(MI_SYS_TaskLock_t *pstTaskLock, char *szLockName);
void _MI_SYS_CheckUnlock(MI_SYS_LockNode_t *pstLockNode);

#ifndef CAM_OS_LIST_PREV_ENTRY
/**
 * list_prev_entry - get the prev element in list
 * @pos:    the type * to cursor
 * @member: the name of the list_head within the struct.
 */
#define CAM_OS_LIST_PREV_ENTRY(pos, member) \
    CAM_OS_LIST_ENTRY((pos)->member.pPrev, typeof(*(pos)), member)
#endif

#ifndef CAM_OS_LIST_FOR_EACH_ENTRY_SAFE_REVERSE
/**
 * list_for_each_entry_safe_reverse - iterate backwards over list safe against removal
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_head within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define CAM_OS_LIST_FOR_EACH_ENTRY_SAFE_REVERSE(pos, n, head, member)       \
    for (pos = CAM_OS_LIST_LAST_ENTRY(head, typeof(*pos), member),      \
        n = CAM_OS_LIST_PREV_ENTRY(pos, member);            \
         &pos->member != (head);                    \
         pos = n, n = CAM_OS_LIST_PREV_ENTRY(n, member))
#endif

#define MI_SYS_LOCK_TEMPLATE(lock, lock_name, func) \
    do \
    { \
        MI_SYS_TaskLock_t *pstTaskLock = NULL; \
        MI_SYS_LockNode_t *pstLockNode = NULL; \
        pstTaskLock = _MI_SYS_FindTaskLockInstance(); \
        _MI_SYS_CheckLock(pstTaskLock, lock_name); \
        pstLockNode = _MI_SYS_CreateNewLockNode(lock_name); \
        snprintf(pstLockNode->szFunc, MI_SYS_LOCK_NAME_LENGTH, __FUNCTION__); \
        pstLockNode->line = __LINE__; \
        pstTaskLock->pstRequestLockNode = pstLockNode; \
        func(lock); \
        pstTaskLock->pstRequestLockNode = NULL; \
        _MI_SYS_AttachLockNodeToList(pstLockNode, pstTaskLock); \
    }while(0);

#define MI_SYS_UNLOCK_TEMPLATE(lock, lock_name, func) \
    do \
    { \
        MI_SYS_TaskLock_t *pstTaskLock = NULL; \
        MI_SYS_LockNode_t *pstLockNode = NULL; \
        func(lock); \
        pstTaskLock = _MI_SYS_FindTaskLockInstance(); \
        pstLockNode = _MI_SYS_FindLockNodeByName(pstTaskLock, lock_name); \
        _MI_SYS_CheckUnlock(pstLockNode); \
        _MI_SYS_ReleaseLockNode(pstTaskLock, pstLockNode); \
    }while(0);

#define MI_SYS_SPIN_LOCK(lock, lock_name) \
    do \
    { \
        MI_SYS_LOCK_TEMPLATE(lock, lock_name, CamOsSpinLock); \
    }while(0);

#define MI_SYS_SPIN_UNLOCK(lock, lock_name) \
    do \
    { \
        MI_SYS_UNLOCK_TEMPLATE(lock, lock_name, CamOsSpinUnlock) \
    }while(0);

#define MI_SYS_DOWN(lock, lock_name) \
    do \
    { \
        MI_SYS_LOCK_TEMPLATE(lock, lock_name, CamOsTsemDown); \
    }while(0);

#define MI_SYS_UP(lock, lock_name) \
    do \
    { \
        MI_SYS_UNLOCK_TEMPLATE(lock, lock_name, CamOsTsemUp); \
    }while(0);

#define MI_SYS_MUTEX_LOCK(lock, lock_name) \
    do \
    { \
        MI_SYS_LOCK_TEMPLATE(lock, lock_name, CamOsMutexLock); \
    }while(0);

#define MI_SYS_MUTEX_UNLOCK(lock, lock_name) \
    do \
    { \
        MI_SYS_UNLOCK_TEMPLATE(lock, lock_name, CamOsMutexUnlock); \
    }while(0);

#ifdef MI_SYS_PROC_FS_DEBUG
int MI_SYS_IMPL_DeadlockInfoProcShow(struct seq_file *m, void *v);
#endif/*#ifdef MI_SYS_PROC_FS_DEBUG*/

#else
#define MI_SYS_SPIN_LOCK(lock, lock_name) \
    do \
    { \
        CamOsSpinLock(lock); \
    }while(0);

#define MI_SYS_SPIN_UNLOCK(lock, lock_name) \
    do \
    { \
        CamOsSpinUnlock(lock); \
    }while(0);

#define MI_SYS_DOWN(lock, lock_name) \
    do \
    { \
        CamOsTsemDown(lock); \
    }while(0);

#define MI_SYS_UP(lock, lock_name) \
    do \
    { \
        CamOsTsemUp(lock); \
    }while(0);

#endif/*#if (defined __KERNEL__) && (defined CONFIG_MI_SYS_ENABLE_DETECT_DEADLOCK)*/

#endif/*#ifndef _MI_SYS_LOCK_H_*/