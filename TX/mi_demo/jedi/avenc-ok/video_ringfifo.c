/*ringbuf .c*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "video_ringfifo.h"

#define NMAX 40

int g_WriteIndex = 0; /*The current place position of the ring buffer*/
int g_ReadIndex = 0; /*The current fetch position of the buffer*/
int g_AllNumOfElements = 0;	 /*The total number of elements in the ring buffer*/

RING_NODE_FRAME g_Ringfifo[NMAX]; //The ring buffer array

#define GET_INDEX(i) (i==(NMAX-1) ? 0 : ++i)

//malloc for an element on the ring buffer 
static char MallocIndex(int index, int size)
{
    //printf("index : %d , size : %d , pFrame : %x \n", index, size, g_Ringfifo[index].pFrame);

    if ((index >= 0) && (index < NMAX))
    {
        if (g_Ringfifo[index].pFrame == NULL)
        {
            g_Ringfifo[index].pFrame = malloc(size);
            g_Ringfifo[index].FrameSize = size;
            return 0;
        }
    }
    printf("MallocIndex error \n");
}


static void FreeIndex(unsigned int index)
{
    if ((index >= 0) && (index < NMAX))
    {
        if (g_Ringfifo[index].pFrame != NULL)
        {
            free(g_Ringfifo[index].pFrame);
            g_Ringfifo[index].pFrame = NULL;
            g_Ringfifo[index].FrameSize = 0;
            return 0;
        }
    }
    printf("FreeIndex error \n");
}

static unsigned int GetAllNumOfElements(void)
{
    //printf("g_AllNumOfElements : %d \n", g_AllNumOfElements);
    return g_AllNumOfElements;
}

void RingInit(void)
{
    g_ReadIndex = 0;
    g_WriteIndex = 0;
    g_AllNumOfElements = 0;

    int i;
    for (i=0; i<NMAX; i++)
    {
        g_Ringfifo[i].pFrame = NULL;
        g_Ringfifo[i].FrameSize = 0;
        g_Ringfifo[i].FrameType = 0;
    }
}

/*Get data from ring buffer*/
unsigned int GetElementFromRing(RING_NODE_FRAME *pGetElementInfo)
{
    //printf("GetElementFromRing \n");
    int ret = 0;

    if ((GetAllNumOfElements() > 0) && (GetAllNumOfElements() <= NMAX))
    {
        if (pGetElementInfo->pFrame != NULL)
        {
            free(pGetElementInfo->pFrame);
            pGetElementInfo->pFrame = NULL;
        }
        pGetElementInfo->pFrame = malloc(g_Ringfifo[g_ReadIndex].FrameSize);
        memcpy(pGetElementInfo->pFrame, g_Ringfifo[g_ReadIndex].pFrame, g_Ringfifo[g_ReadIndex].FrameSize);
        pGetElementInfo->FrameSize = g_Ringfifo[g_ReadIndex].FrameSize;
        pGetElementInfo->FrameType = g_Ringfifo[g_ReadIndex].FrameType;
        FreeIndex(g_ReadIndex);

        g_ReadIndex = GET_INDEX(g_ReadIndex);

        g_AllNumOfElements--;

        ret = pGetElementInfo->FrameSize;
    }
    else
    {
        //printf("The Ring Buffer is Empty !! \n");
        ret = 0;
    }

    return ret;
}

//Put H264 data to the ring buffer 
unsigned int PutH264DataToBuffer(unsigned char *pFrame, unsigned int Size, int Type)
{
    //printf("\n\n----------------------------PutH264DataToBuffer-----------------------------\n\n");
    //printf("0x%x 0x%x 0x%x 0x%x 0x%x", pFrame[0], pFrame[1], pFrame[2], pFrame[3], pFrame[4]);
    //printf("size : %d Type: %d \n", Size, Type);
    int ret = 0;

#if 1
    if ((GetAllNumOfElements() < NMAX) && (GetAllNumOfElements() >= 0))
    {
        if (5 == Type)
        {
            //printf("this is I frame \n");
            g_Ringfifo[g_WriteIndex].FrameType = FRAME_TYPE_I;
            MallocIndex(g_WriteIndex, Size);
            memcpy(g_Ringfifo[g_WriteIndex].pFrame, pFrame, Size);
        }
        else
        {
            //printf("this is P frame \n");
            g_Ringfifo[g_WriteIndex].FrameType = FRAME_TYPE_P;
            MallocIndex(g_WriteIndex, Size);
            memcpy(g_Ringfifo[g_WriteIndex].pFrame, pFrame, Size);
        }

        g_WriteIndex = GET_INDEX(g_WriteIndex);
        
        g_AllNumOfElements++;
        ret = Size;
    }
    else
    {
        printf("The Ring Buffer is full !! \n");
        ret = 0;
    }
#endif

    return ret;
}
