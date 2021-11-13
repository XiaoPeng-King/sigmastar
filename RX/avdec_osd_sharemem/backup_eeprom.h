#ifndef __IIC_H_
#define __IIC_H_

#include "sharemem.h"

int InitShareMemFromE2prom(SHARE_MEM *pShareMemory);
int WriteConfigIntoE2prom(SHARE_MEM *pShareMemory);

#endif



