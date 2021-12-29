#ifndef __UART_H_
#define __UART_H_


int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop);
int open_port(int fd,int comport);

int uart_write(unsigned char uart_port, unsigned int bitrate, char *pbuffer, unsigned char buffer_len);
int uart_read(unsigned char uart_port, unsigned int bitrate, char *pbuffer, unsigned char buffer_len);

#endif