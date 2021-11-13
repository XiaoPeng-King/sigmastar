#ifndef __IIC_H_
#define __IIC_H_

int i2c_byte_write(int fd,unsigned char slave_addr, unsigned char reg_addr, unsigned char value);
int i2c_bytes_write(int fd,unsigned char slave_addr, unsigned char reg_addr, unsigned long int len, unsigned char *buf);
int i2c_byte_read(int fd, unsigned char slave_addr, unsigned char reg_addr,  unsigned char *value);
int i2c_bytes_read(int fd, unsigned char slave_addr, unsigned char reg_addr, unsigned long int len, unsigned char *pdata);

#endif