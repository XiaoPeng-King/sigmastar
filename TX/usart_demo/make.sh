arm-linux-gnueabihf-9.1.0-gcc -Wall -g -DLINUX_OS -I./ -Wl,--gc-sections main_send.c  usart.c -o usart_send
arm-linux-gnueabihf-9.1.0-gcc -Wall -g -DLINUX_OS -I./ -Wl,--gc-sections main_recv.c  usart.c -o usart_recv

cp usart_send /mnt/nfs/TX
cp usart_recv /mnt/nfs/RX


