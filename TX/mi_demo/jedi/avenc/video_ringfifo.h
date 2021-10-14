#ifndef __VIDEO_RINGFIFO_H_
#define __VIDEO_RINGFIFO_H_


enum H264_FRAME_TYPE {FRAME_TYPE_I, FRAME_TYPE_P, FRAME_TYPE_B};

typedef struct{
    unsigned char *pFrame;
	unsigned int FrameType; //
    unsigned int FrameSize; //frame size 
}RING_NODE_FRAME;

unsigned int PutH264DataToBuffer(unsigned char *pFrame,unsigned int size, int Type);
unsigned int GetElementFromRing(RING_NODE_FRAME *pGetElementInfo);
void RingInit(void);
#endif
