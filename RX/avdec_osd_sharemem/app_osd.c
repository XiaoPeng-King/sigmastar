#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include "font_8x16.c"
#include "main.h"

//extern SYSTEM_ATTR_s g_system_attr;
static char OSD_state = 1;

int fd_fb;
struct fb_var_screeninfo var;	/* Current var */
int screen_size;
unsigned char *fbmem;
unsigned int line_width;
unsigned int pixel_width;
const unsigned char* fontdata = fontdata_8x16;

static void lcd_put_pixel(int x, int y, unsigned int color){
	unsigned char *pen_8 = fbmem+y*line_width+x*pixel_width;
	unsigned short *pen_16;
	unsigned int *pen_32;	

	unsigned int red, green, blue;	

	pen_16 = (unsigned short *)pen_8;
	pen_32 = (unsigned int *)pen_8;

	switch (var.bits_per_pixel){
		case 8:{
			*pen_8 = color;
			break;
		}
		case 16:{
			/* 565 */
			red   = (color >> 16) & 0xff;
			green = (color >> 8) & 0xff;
			blue  = (color >> 0) & 0xff;
			color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
			*pen_16 = color;
			break;
		}
		case 32:{
			*pen_32 = color;
			break;
		}
		default:{
			printf("can't surport %dbpp\n", var.bits_per_pixel);
			break;
		}
	}
}

static void lcd_put_ascii(int x, int y, unsigned char c){
	unsigned char *dots = (unsigned char *)&fontdata[c*16];//找到ASCII字符在fontdata数组中的点阵起始位置
	int i, b;
	unsigned char byte;

	for (i = 0; i < 16; i++){//遍历点阵的每一行数据
		byte = dots[i];
		for (b = 7; b >= 0; b--){///从左到右一次判断对应的像素是否应该点亮或者熄灭
			if (byte & (1<<b)){
				/* show */
				lcd_put_pixel(x+7-b, y+i, 0xffffffff); /* 白  x+7-b 表示像素的x坐标*/ 
				//lcd_put_pixel(x+7-b, y+i, 255<<11|255<<5|255);
			}else{
				/* hide */
				//lcd_put_pixel(x+7-b, y+i, 255<<11|255<<5|255); /* 黑 */
			}
		}
	}
}

int osd_disable(void)
{
	if (1 == OSD_state)
		OSD_state = 0;
	else
		return 0;

	printf("osd disable !\n");
	fd_fb = open("/dev/fb0", O_RDWR);
	if (fd_fb < 0){
		printf("can't open /dev/fb0\n");
		return -1;
	}
	if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &var)){///获取屏幕可变信息
		printf("can't get var\n");
		return -1;
	}

	line_width  = var.xres * var.bits_per_pixel / 8;//一行像素所需要的字节数
	pixel_width = var.bits_per_pixel / 8;//一个像素需要的字节数
	screen_size = var.xres * var.yres * var.bits_per_pixel / 8;//一帧画面所需要的字节数
	//printf("x: %d, y: %d \n", var.xres, var.yres);
	//printf("screen size : %d \n", screen_size);
	//printf("bits_per_pixel : %d.\n",var.bits_per_pixel);

	fbmem = (unsigned char *)mmap(NULL , screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);//映射framebuffer
	if (fbmem == (unsigned char *)-1){
		printf("can't mmap\n");
		return -1;
	}

	/* 清屏: 全部设为黑色 */
	memset(fbmem, 0, screen_size);//设置黑屏

	munmap(fbmem , screen_size);
	close(fd_fb);
	return 0;
}

int osd_display(unsigned int x, unsigned int y, char *str)
{
	if (0 == OSD_state)
		OSD_state = 1;
	else
		return 0;

	printf("osd display : %s !\n", str);
	fd_fb = open("/dev/fb0", O_RDWR);
	if (fd_fb < 0){
		printf("can't open /dev/fb0\n");
		return -1;
	}
	if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &var)){///获取屏幕可变信息
		printf("can't get var\n");
		return -1;
	}

	line_width  = var.xres * var.bits_per_pixel / 8;//一行像素所需要的字节数
	pixel_width = var.bits_per_pixel / 8;//一个像素需要的字节数
	screen_size = var.xres * var.yres * var.bits_per_pixel / 8;//一帧画面所需要的字节数
	//printf("x: %d, y: %d \n", var.xres, var.yres);
	//printf("screen size : %d \n", screen_size);
	//printf("bits_per_pixel : %d.\n",var.bits_per_pixel);

	fbmem = (unsigned char *)mmap(NULL , screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);//映射framebuffer
	if (fbmem == (unsigned char *)-1){
		printf("can't mmap\n");
		return -1;
	}

	/* 清屏: 全部设为黑色 */
	memset(fbmem, 0, screen_size);//设置黑屏

	unsigned short int* pdest=fbmem + x + y; 
	for(int r=0;r<strlen(str)*8;r++){
		for(int c=0; c<8; c++){
			pdest++;
		}
	}

	for(int i=0; i<strlen(str); i++){
		lcd_put_ascii(x + i*8, y, str[i]);
		//printf("x: %d , y: %d , str : %c \n", x + i*8, y, str[i]);
	}
	
	munmap(fbmem , screen_size);
	close(fd_fb);
	return 0;
}
