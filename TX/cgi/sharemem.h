#ifndef __SHAREMEM_H_
#define __SHAREMEM_H

#include <stdio.h>
#include <sys/shm.h>
#include "Setting.h"
#include <unistd.h>

typedef struct{
	unsigned char ucUpdateFlag; //1:updated
	unsigned char ucModeApplyFlag; //1:apply
	unsigned char ucInfoDisplayFlag; //1:send flag of display information to RX
	unsigned char ucCurrentMode;
	unsigned char ucControlBoxFlag;
	unsigned char ucOnlineNum[256];
	RUN_STATUS sm_run_status;
	ETH_SETTING sm_eth_setting;
	GROUP_PACK_S sm_group_pack; //wang
	GROUP_RENAME_S sm_group_rename;
	MODE_RENAME_S sm_mode_rename;
}SHARE_MEM;
SHARE_MEM *share_mem;

typedef struct{
unsigned int uiFileLen;
unsigned int uiWriteLen;
}SHARE_UPLOAD;

SHARE_UPLOAD *share_upload;

int InitShareMem(void);
#endif
