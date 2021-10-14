#ifndef __VIDEO_RINGFIFO_H_
#define __VIDEO_RINGFIFO_H_


enum H264_FRAME_TYPE {FRAME_TYPE_I, FRAME_TYPE_P, FRAME_TYPE_B};

typedef struct _H264_APPEND_INFO
{
        //audio
        unsigned int fs;
        unsigned int audio_bits;
        unsigned int chns;
        //unsigned int audio_handle;
        //video
        unsigned int width;
        unsigned int height;
        unsigned int src_width;
        unsigned int src_height;
        unsigned char frameRate;
        unsigned char refreshRate;
        unsigned char interlace;
}
H264_APPEND_INFO_s;

typedef struct{
    unsigned char *pFrame;
	unsigned int FrameType; //
    unsigned int FrameSize; //frame size 
}RING_NODE_FRAME;

void update_init_info(void);
unsigned int PutH264DataToBuffer(unsigned char *pFrame,unsigned int size, int Type);
unsigned int GetElementFromRing(RING_NODE_FRAME *pGetElementInfo);
void RingInit(void);
#endif
