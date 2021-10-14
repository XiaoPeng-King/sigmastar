#ifndef _SHAREMEM_H
#define _SHAREMEM_H

#include <stdio.h>
#include <sys/shm.h>
#include "Setting.h"
#include <unistd.h>


typedef struct{
	unsigned char ucUpdateFlag; //1:updated
	unsigned int uuid;
	RUN_STATUS sm_run_status;
	ETH_SETTING sm_eth_setting;
	ENCODER_SETTING sm_encoder_setting;
	CEC_CONTROL_S cec_control; //cec control
}SHARE_MEM;

SHARE_MEM *share_mem;

typedef struct{
unsigned int uiFileLen;
unsigned int uiWriteLen;
}SHARE_UPLOAD;

SHARE_UPLOAD *share_upload;

int InitShareMem(void);

#endif
