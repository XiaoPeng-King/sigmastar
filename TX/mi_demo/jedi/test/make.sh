#!/bin/sh
arm-linux-gnueabihf-9.1.0-gcc -o iic iic.c
arm-linux-gnueabihf-9.1.0-gcc -o uart uart.c
arm-linux-gnueabihf-9.1.0-gcc -o test test.c

cp iic uart test /mnt/nfs/TX