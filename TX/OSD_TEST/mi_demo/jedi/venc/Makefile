TOPDIR=/home/king/Work/sigmastar/HOT/TX/TX_OSD文字_20210729/mi_demo/
NFS_DIR=/nfs_root

CC=arm-linux-gnueabihf-9.1.0-gcc
STRIP=arm-linux-gnueabihf-9.1.0-strip

INCLUDEFLAGS := -I$(TOPDIR)/jedi/internal/common
INCLUDEFLAGS += -I$(TOPDIR)/jedi/internal/rgn
INCLUDEFLAGS += -I$(TOPDIR)/mi_include/release/include
INCLUDEFLAGS += -I$(TOPDIR)/mi_include/drivers/sstar/include

CFLAGS = -Wall -g -mthumb -pipe -fPIC -DLINUX_OS -Wl,--gc-sections 

LIBS := -lmi_sys
LIBS += -lmi_common
LIBS += -lmi_isp
LIBS += -lmi_vpe
LIBS += -lmi_sensor
LIBS += -lmi_venc
LIBS += -lmi_vif
LIBS += -lmi_rgn
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
all:prog_venc_264 prog_venc_265

prog_venc_264:st_main_venc_264.c
	$(CC) $(CFLAGS) $(INCLUDEFLAGS) $(LIBS) $(LIBS_PATH)  $< -o $@ 
	$(STRIP) --strip-unneeded $@

prog_venc_265: st_main_venc_265.o
	$(CC) $(CFLAGS) $(INCLUDEFLAGS) $(LIBS) $(LIBS_PATH) $< /home/king/Work/sigmastar/HOT/TX/TX_OSD文字_20210729/mi_demo/jedi/lib/libst_rgn.a -o $@
	$(STRIP) --strip-unneeded $@

st_main_venc_265.o:st_main_venc_265.c
	$(CC) $(CFLAGS) $(INCLUDEFLAGS) -c -ffunction-sections -fdata-sections $< -o $@

install:
	cp prog_venc_264 $(NFS_DIR) -f
	cp prog_venc_265 $(NFS_DIR) -f
	cp venc_config_264.json $(NFS_DIR) -f
	cp venc_config_265.json $(NFS_DIR) -f

clean:
	rm -rf prog_venc_264 prog_venc_265 *.o
