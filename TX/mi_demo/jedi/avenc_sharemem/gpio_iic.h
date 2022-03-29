#ifndef	 	GPIO_IIC_H_
#define 	GPIO_IIC_H_

void i2c_gpio_init(void);
void  gpio_i2c_start(void);
void  gpio_i2c_end(void);
unsigned char gpio_i2c_read_ack(void);
void gpio_i2c_send_ack(void);
unsigned char gpio_i2c_read_byte(void);
void gpio_i2c_write_byte(unsigned char byte);


#endif 

