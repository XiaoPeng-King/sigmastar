#include "audio_ringbuffer.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/syscall.h>
#include <errno.h>

//for aac encode
typedef struct
{
	unsigned char *membase;
	unsigned int buffer_size;
	unsigned int read_offset_bytes;
	unsigned int write_offset_bytes;
}RingBufferBK;

#define MIN(X, Y)	(((unsigned int)(X) > (unsigned int)(Y))?  (Y) : (X))
#define RINGBUFFER_SIZE		1024*1024
static RingBufferBK *Rb = NULL;
//FILE *pAudio = NULL;
static int InitRingBuffer()
{
	
	if (NULL != Rb)
		return 0;
	printf("InitRingBuffer\n");
	//pAudio = fopen("./Audio.pcm", "w+");
	
	if((RINGBUFFER_SIZE & (RINGBUFFER_SIZE - 1 )) != 0)		
	{
		printf("Ring Buffer Size Must Be Several Times The Power Of 2\n");
		return -1;
	}

	Rb = (RingBufferBK *)malloc(sizeof(RingBufferBK));
	if(NULL == Rb)
	{
		printf("fail to malloc memory for struct Rb\n");
		return -1;
	}

	Rb->buffer_size = RINGBUFFER_SIZE;
	Rb->read_offset_bytes = 0;
	Rb->write_offset_bytes = 0;
	Rb->membase = (unsigned char *)malloc(Rb->buffer_size);
	if(NULL == Rb->membase)
	{
		printf("fail to malloc memory for RingBuffer, requre size:%d\n",  Rb->buffer_size);
		free(Rb);
		Rb = NULL;
		return -1;
	}

	return 0;
}

static int  FreeRingBuffer(void)
{
	if(Rb == NULL)
	{
		printf("Rb haven't been Init Yet\n");
		return -1;
	}

	free(Rb->membase);
	free(Rb);
	Rb = NULL;

	//return 0;
}

static int CountRingBufferUsed()
{
	RingBufferBK *rb;
	unsigned int count;
	unsigned int read_ptr,write_ptr;
    
    if(Rb == NULL)
        return 0;
    
    rb = Rb;
	read_ptr = rb->read_offset_bytes & (rb->buffer_size - 1);
	write_ptr = rb->write_offset_bytes & (rb->buffer_size - 1);
	if(rb->read_offset_bytes == rb->write_offset_bytes)
		count = 0;
	else if(write_ptr > read_ptr)
		count = write_ptr - read_ptr;
	else 
		count = rb->buffer_size + write_ptr - read_ptr;

	return count;
}

static int CountRingBufferFree()
{
	RingBufferBK *rb;
	int size;
	
    if(Rb == NULL)
        return 0;
    
	rb = Rb;
	size = rb->buffer_size - CountRingBufferUsed();

	return size;
}

static int PushToRingBuffer(unsigned char *buffer, int dlen)
{
	unsigned int write_ptr, count;

	if(Rb == NULL)
	{
		printf("Rb haven't been Init Yet\n");
		return -1;
	}
	if(CountRingBufferFree() < dlen)
    {
        printf("ringbuffer is full\n");
		//FreeRingBuffer();
        
		return -1;
    }
        
	write_ptr = Rb->write_offset_bytes & (Rb->buffer_size - 1);
	count = MIN(dlen, Rb->buffer_size - Rb->write_offset_bytes%Rb->buffer_size);
	memcpy(Rb->membase + write_ptr, buffer, count);
	memcpy(Rb->membase, buffer + count, dlen - count);	
	Rb->write_offset_bytes += dlen;

	return dlen;
}

static int PullFromRingBuffer(unsigned char *buffer, int dlen)
{
	unsigned int read_ptr, count;

	if(Rb == NULL)
	{
		printf("Rb haven't been Init Yet\n");
		return -1;
	}
	if(CountRingBufferUsed() == 0)
		return 0;
	dlen = MIN(CountRingBufferUsed(), dlen);

	read_ptr = Rb->read_offset_bytes & (Rb->buffer_size - 1);
	count = MIN(dlen, Rb->buffer_size - Rb->read_offset_bytes%Rb->buffer_size);
	         
	memcpy(buffer, Rb->membase + read_ptr, count);	
	memcpy(buffer + count, Rb->membase, dlen - count);
	Rb->read_offset_bytes += dlen;
	//printf("dlen : %d \n", dlen);
	return dlen;
}

int RingGetByteStreamMemoryBufferCnt()
{
	int cnt = CountRingBufferUsed();
	//printf("cnt : %d \n", cnt);
	return cnt;
}

int RingPullFromByteStreamMemoryBuffer(unsigned char * buff, unsigned int size)
{
    return PullFromRingBuffer(buff, size);
}

int RingPCMPush(unsigned char * buff, unsigned int size)
{
	char ret = 0;
	InitRingBuffer();
	//printf("push size : %d \n", size);
	//ret = fwrite(buff, size, 1, pAudio);
	//printf("write : %d \n", ret);
	ret = PushToRingBuffer(buff, size);
	if (ret < 0)
	{
		printf("\n PushToRingBuffer fail \n");
		return -1;
	}
	return 0;
}

