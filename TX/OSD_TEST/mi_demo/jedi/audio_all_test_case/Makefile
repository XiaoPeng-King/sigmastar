TOPDIR=/home/lqs/work_2/TX/Ri_Dian/SDK/mi_demo
NFS_DIR=/nfs_root

CC=arm-linux-gnueabihf-9.1.0-gcc
STRIP=arm-linux-gnueabihf-9.1.0-strip

INCLUDEFLAGS := -I$(TOPDIR)/jedi/internal/common
INCLUDEFLAGS += -I$(TOPDIR)/mi_include/release/include
INCLUDEFLAGS += -I$(TOPDIR)/mi_include/drivers/sstar/include

CFLAGS = -Wall -g -mthumb -pipe -fPIC -DLINUX_OS -Wl,--gc-sections 

LIBS := -lmi_sys
LIBS += -lmi_common
LIBS += -lmi_isp
LIBS += -lmi_vpe
LIBS += -lmi_sensor
LIBS += -lmi_ai
LIBS += -lmi_ao
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

all:prog_audio

prog_audio:audio_all_test_case.c
	$(CC) $(CFLAGS) $(INCLUDEFLAGS) $(LIBS) $(LIBS_PATH) $< -o $@ 
	$(STRIP) --strip-unneeded $@

install:
	cp prog_audio $(NFS_DIR) -f

clean:
	rm -rf prog_audio