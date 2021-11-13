#!/bin/sh
arm-linux-gnueabihf-gcc -o iic iic.c
arm-linux-gnueabihf-gcc -o uart uart.c
arm-linux-gnueabihf-gcc -o test test.c

cp iic uart test /mnt/nfs/RX