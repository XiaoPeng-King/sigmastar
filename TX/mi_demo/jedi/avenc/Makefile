TOPDIR=/home/king/Work/sigmastar/HOT/TX/mi_demo
NFS_DIR=/mnt/nfs
#COMMOM_DIR = /home/king/Work/COMMOM

CC=arm-linux-gnueabihf-9.1.0-gcc
STRIP=arm-linux-gnueabihf-9.1.0-strip

INCLUDEFLAGS := -I$(TOPDIR)/jedi/internal/common
INCLUDEFLAGS += -I$(TOPDIR)/mi_include/release/include
INCLUDEFLAGS += -I$(TOPDIR)/mi_include/drivers/sstar/include
#INCLUDEFLAGS += -I$(COMMOM_DIR)

CFLAGS = -Wall -g -mthumb -pipe -fPIC -DLINUX_OS -Wl,--gc-sections 

LIBS := -lmi_sys
LIBS += -lmi_common
LIBS += -lmi_isp
LIBS += -lmi_vpe
LIBS += -lmi_sensor
LIBS += -lmi_ai
LIBS += -lmi_ao
LIBS += -lmi_venc
LIBS += -lmi_vif
LIBS += -lcus3a
LIBS += -lispalgo
LIBS += -lcam_fs_wrapper
LIBS += -lcam_os_wrapper
LIBS += -lrt
LIBS += -lpthread
LIBS += -lm
LIBS += -ldl

LIBS_PATH := -L$(TOPDIR)/mi_libs/dynamic
LIBS_PATH += -L$(TOPDIR)/ex_libs/dynamic

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

TARGET := main

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) -o $@ $(OBJ) $(CFLAGS) $(INCLUDEFLAGS) $(LIBS) $(LIBS_PATH)

$(OBJ) : %.o:%.c
	$(CC) $(CFLAGS) $(INCLUDEFLAGS) -c $< -o $@
	
install:
	cp $(TARGET) $(NFS_DIR) -f

clean:
	rm -rf $(OBJ) $(TARGET)
