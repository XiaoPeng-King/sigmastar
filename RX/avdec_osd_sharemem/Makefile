NFS = /mnt/nfs/RX
CROSS_COMPILE ?=arm-linux-gnueabihf-
CC  = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
AR  = $(CROSS_COMPILE)ar

ENABLE_HDMI = 1
USE_FFMPEG  = 0

COM_FLAGS = -Wall -O2 -fPIC -mcpu=cortex-a9 -mfpu=neon-fp16 -mfloat-abi=hard -mthumb-interwork -marm
C_FLAGS	   = $(COM_FLAGS) #-std=c11

INCLUDES  += -I. -I./sstar/include -I./osd_lib/inc -I./common 

TARGET_NAME  = main_rx

C_SRCS    =  $(wildcard *.c) common/verify_gfx.c common/blitutil.c #sstardisp.c main.c app_rtp.c
C_OBJS = $(patsubst %.c, %.o, $(C_SRCS))#main.o sstardisp.o app_rtp.o

$(warning $(C_OBJS))

LIB_PATH  = -L. -L./sstar/lib -L./osd_lib/lib
LIB_NAME += -lm -lmi_vdec -lmi_sys -lmi_disp -lmi_ao -lmi_common -ldl  
LIB_NAME += -lmi_gfx -ljpeg -lz -lmi_panel


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
install:
	sudo cp $(TARGET_NAME) $(NFS)

clean:
	@rm -Rf $(CPP_OBJS)
	@rm -f $(C_OBJS)
	@rm -Rf $(TARGET_NAME)
	@rm -f *.pcm
