TOPDIR=/home/lqs/work_2/TX/Ri_Dian/SDK/mi_demo
LIB_DIR=/home/lqs/work_2/TX/Ri_Dian/SDK/mi_demo/jedi/lib

CC=arm-linux-gnueabihf-9.1.0-gcc
STRIP=arm-linux-gnueabihf-9.1.0-strip
AR=arm-linux-gnueabihf-9.1.0-ar

INCLUDEFLAGS := -I$(TOPDIR)/jedi/internal/common
INCLUDEFLAGS += -I$(TOPDIR)/jedi/internal/rgn
INCLUDEFLAGS += -I$(TOPDIR)/mi_include/release/include
INCLUDEFLAGS += -I$(TOPDIR)/mi_include/drivers/sstar/include

CFLAGS_AR = sq
CFLAGS := -Wall -g -mthumb -pipe -fPIC -DLINUX_OS
CFLAGS_F :=  -ffunction-sections -fdata-sections

LIBS_PATH := -L$(TOPDIR)/mi_libs/dynamic
LIBS_PATH += -L$(TOPDIR)/ex_libs/dynamic

all:libst_rgn.a

libst_rgn.a:st_rgn.o dot_matrix_font.o
	$(AR) $(CFLAGS_AR) $@  st_rgn.o dot_matrix_font.o

st_rgn.o:st_rgn.c
	$(CC) $(CFLAGS) $(INCLUDEFLAGS) $(CFLAGS_F) -c $< 

dot_matrix_font.o:dot_matrix_font.c
	$(CC) $(CFLAGS) $(INCLUDEFLAGS) $(CFLAGS_F) -c $< 

install:
	cp *.a $(LIB_DIR) -f

clean:
	rm -rf *.o *.a

