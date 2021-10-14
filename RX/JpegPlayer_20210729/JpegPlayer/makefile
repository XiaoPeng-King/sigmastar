PWD=`pwd`
SDK_LIBS=./SDK_LIBS
SDK_LIBS_STATIC=./SDK_LIBS_STATIC
PLAT_DEPENDENT_LIB=-lmi_gfx -lmi_sys -lmi_common -ldl -lmi_hdmi 
PLAT_DEPENDENT_LIB_EX= -lmi_disp -lmi_panel 
MINIGUI_LIBS= -ljpeg -lz 
SDK_INCS=./SDK_INCS
UAPI_INCS=./UAPI_INCS
INCLUDES=./common
LDFLAGS+=-L$(SDK_LIBS)  -L$(SDK_LIBS_STATIC)
LDFLAGS+=-L$(PWD)/lib -L/usr/local/lib
STDLIBS:=  -lstdc++ -ldl  -lpthread -lm 
LIBS:=-Wl,-Bdynamic ${MINIGUI_LIBS} ${PLAT_DEPENDENT_LIB_EX} ${PLAT_DEPENDENT_LIB} -Wl,-Bdynamic $(STDLIBS)
CFLAGS:=-rdynamic -funwind-tables -I. -I$(SDK_INCS) -I$(PWD)/inc -I$(UAPI_INCS) -I$(INCLUDES)
#-I$(PWD)/library/include -I$(PWD)/library/include/ctrl -I$(PWD)/library/include/libpng16/ -I/usr/local/include -I$(UAPI_INCS)
CC:=arm-linux-gnueabihf-gcc


LOGO_SRC:=logo.c  sstardisp.c bmp.c  jpeg.c common/verify_gfx.c common/blitutil.c
LOGO:=logo

.PHONY : all clean


all:
	$(CC) -D_GNU_SOURCE -o  $(LOGO)  $(LOGO_SRC) $(CFLAGS) $(LDFLAGS) $(LIBS) 
clean:
	rm -f $(patsubst %.c,%.o,$(LOGO_SRC))
	rm -f $(LOGO)
install:
	@echo done!
