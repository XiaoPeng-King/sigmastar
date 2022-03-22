rm *.cgi
echo "Buinding"
#index.html
arm-linux-gnueabihf-9.1.0-gcc -o get_status.cgi get_status.c

#SetNet.html
arm-linux-gnueabihf-9.1.0-gcc -o get_net.cgi get_net_control.c
arm-linux-gnueabihf-9.1.0-gcc -o set_net.cgi set_net_control.c

#SetMedia.html
#arm-linux-gnueabihf-9.1.0-gcc -o get_codec.cgi get_codec.c
#arm-linux-gnueabihf-9.1.0-gcc -o set_protocol.cgi set_protocol.c
#arm-linux-gnueabihf-9.1.0-gcc -o set_codec.cgi set_codec.c

#System.html
arm-linux-gnueabihf-9.1.0-gcc -o get_sys.cgi get_sys.c
#arm-linux-gnueabihf-9.1.0-gcc -o sys_reboot.cgi sys_reboot.c
#arm-linux-gnueabihf-9.1.0-gcc -o sys_default.cgi sys_default.c
arm-linux-gnueabihf-9.1.0-gcc -o get_bar.cgi get_bar.c
arm-linux-gnueabihf-9.1.0-gcc -o upload.cgi test.c

#scene
arm-linux-gnueabihf-9.1.0-gcc -o scene_apply.cgi scene_apply.c
arm-linux-gnueabihf-9.1.0-gcc -o scene_save.cgi scene_save.c
arm-linux-gnueabihf-9.1.0-gcc -o scene_rename.cgi scene_rename.c

#display information
arm-linux-gnueabihf-9.1.0-gcc -o infoDisplay.cgi infoDisplay.c

#upload
arm-linux-gnueabihf-9.1.0-gcc -o upload_web.cgi upload_web.c
#arm-linux-gnueabihf-9.1.0-gcc -o upload_ko.cgi upload_ko.c

#main.html
arm-linux-gnueabihf-9.1.0-gcc -o onloading.cgi onloading.c
arm-linux-gnueabihf-9.1.0-gcc -o changeModeName.cgi changeModeName.c

#change name
arm-linux-gnueabihf-9.1.0-gcc -o changeRxName.cgi changeRxName.c
arm-linux-gnueabihf-9.1.0-gcc -o changeTxName.cgi changeTxName.c
arm-linux-gnueabihf-9.1.0-gcc -o changeMulticast.cgi changeMulticast.c

#get name
arm-linux-gnueabihf-9.1.0-gcc -o get_txRename.cgi get_txRename.c
arm-linux-gnueabihf-9.1.0-gcc -o get_rxRename.cgi get_rxRename.c
arm-linux-gnueabihf-9.1.0-gcc -o get_multicast.cgi get_multicast.c

#mode
arm-linux-gnueabihf-9.1.0-gcc -o modeStateNow.cgi modeStateNow.c
arm-linux-gnueabihf-9.1.0-gcc -o saveCurrentMode.cgi saveCurrentMode.c


cp *.cgi ../www/cgi-bin
echo "Building OK !!"
