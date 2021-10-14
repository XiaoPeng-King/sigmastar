编译方法：
1.make clean;make 即可

使用方法：
1.将编译出来的logo和logo.jpg以及lib文件夹拷贝到板子同一个目录；
2.将lib目录export到环境变量：export LD_LIBRARY_PATH=/lib:$PWD/lib:$LD_LIBRARY_PATH
3.运行logo: ./logo

注意事项：
1.demo默认使用的是1024x600的屏参和jpeg图片，如果要换panel对应的屏参和jpeg图片都要替换；
2./config/fbdev.ini里面的FB_WIDTH和FB_HEIGHT需要保持跟屏幕分辨率一致
	FB_WIDTH = 1024
	FB_HEIGHT = 600
3.如果要测试GFX 90度反转功能，把TEST_GFX设置为1，同时logo_480_800.jpg改为logo.jpg,/config/fbdev.ini里的FB_WIDTH和FB_HEIGHT还是变回800和480。
