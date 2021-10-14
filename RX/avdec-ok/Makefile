CROSS_COMPILE ?=arm-linux-gnueabihf-
CC  = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
AR  = $(CROSS_COMPILE)ar

ENABLE_HDMI = 1
USE_FFMPEG  = 0

COM_FLAGS = -Wall -O2 -fPIC -mcpu=cortex-a9 -mfpu=neon-fp16 -mfloat-abi=hard -mthumb-interwork -marm
C_FLAGS	   = $(COM_FLAGS) #-std=c11

INCLUDES  += -I. -I./sstar/include

TARGET_NAME  = main

C_SRCS    =  $(wildcard *.c)#sstardisp.c main.c app_rtp.c
C_OBJS = $(patsubst %.c, %.o, $(C_SRCS))#main.o sstardisp.o app_rtp.o

LIB_PATH  = -L. -L./sstar/lib
LIB_NAME += -lm -lmi_vdec -lmi_sys -lmi_disp -lmi_ao -lmi_common -ldl


ifeq ($(ENABLE_HDMI), 1)
LIB_PATH += -lmi_hdmi
else
LIB_NAME += -lmi_panel
endif

all:$(TARGET_NAME)

$(TARGET_NAME): $(C_OBJS)
	$(CC) -o $@ $(C_OBJS) $(LIB_PATH) $(LIB_NAME) -lm -lpthread

$(C_OBJS) : %.o:%.c
	$(CC) -DENABLE_HDMI=$(ENABLE_HDMI) $(C_FLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

#main.o : main.c
#	$(CC) -DENABLE_HDMI=$(ENABLE_HDMI) $(C_FLAGS) $(INCLUDES) -c $< 

#sstardisp.o : sstardisp.c sstardisp.h
#	$(CC) -DENABLE_HDMI=$(ENABLE_HDMI) $(C_FLAGS) $(INCLUDES) -c $<  

#app_rtp.o : app_rtp.c app_rtp.h
#	$(CC) $(C_FLAGS) $(INCLUDES) -c $< 

clean:
	@rm -Rf $(CPP_OBJS)
	@rm -f $(C_OBJS)
	@rm -Rf $(TARGET_NAME)
	@rm *.pcm
