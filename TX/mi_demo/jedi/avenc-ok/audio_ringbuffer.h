#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_
#ifdef __cplusplus
extern "C" {
#endif
int RingPCMPush(unsigned char * buff, unsigned int size);
int RingPullFromByteStreamMemoryBuffer(unsigned char * buff, unsigned int size);
int RingGetByteStreamMemoryBufferCnt();
#ifdef __cplusplus
}
#endif

#endif
