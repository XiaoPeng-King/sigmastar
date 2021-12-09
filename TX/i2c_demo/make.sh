arm-linux-gnueabihf-9.1.0-gcc -Wall -g -DLINUX_OS -I/home/lqs/work_2/TX/Ri_Dian/sdk/mi_demo/mi_include/release/include -I/home/lqs/work_2/TX/Ri_Dian/sdk/mi_demo/mi_include/drivers/sstar/include -Wl,--gc-sections ./i2c_test.c -ldl -L/home/lqs/work_2/TX/Ri_Dian/sdk/mi_demo/mi_libs/dynamic -L/home/lqs/work_2/TX/Ri_Dian/sdk/mi_demo/ex_libs/dynamic -o i2c_test

cp i2c_test /mnt/nfs/TX 


