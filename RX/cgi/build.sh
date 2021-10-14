rm *.cgi
echo "Buinding"


arm-linux-gnueabihf-gcc -o get_status.cgi get_status.c
arm-linux-gnueabihf-gcc -o get_net.cgi get_net.c
arm-linux-gnueabihf-gcc -o set_net.cgi set_net.c
arm-linux-gnueabihf-gcc -o get_codec.cgi get_codec.c
arm-linux-gnueabihf-gcc -o set_codec.cgi set_codec.c
arm-linux-gnueabihf-gcc -o get_sys.cgi get_sys.c

arm-linux-gnueabihf-gcc -o sys_default.cgi sys_default.c

arm-linux-gnueabihf-gcc -o sys_reboot.cgi sys_reboot.c
arm-linux-gnueabihf-gcc -o upload.cgi test.c
arm-linux-gnueabihf-gcc -o get_bar.cgi get_bar.c
arm-linux-gnueabihf-gcc -o get_sys.cgi get_sys.c
arm-linux-gnueabihf-gcc -o set_net.cgi set_net.c

arm-linux-gnueabihf-gcc -o upload_web.cgi upload_web.c
arm-linux-gnueabihf-gcc -o upload_ko.cgi upload_ko.c

arm-linux-gnueabihf-gcc -o onloading.cgi onloading.c

#************************ cec ***************************
arm-linux-gnueabihf-gcc -o cec_on.cgi cec_on.c
arm-linux-gnueabihf-gcc -o cec_off.cgi cec_off.c
#*********************************************************

cp *.cgi ../www/cgi-bin

echo "Building OK !!"