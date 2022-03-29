#ifndef __GPIO_H_
#define __GPIO_H_


int set_gpio_value(const unsigned char gpio, unsigned char value);
char get_gpio_value(const unsigned char gpio);



unsigned char IP_check(void);
void hdmi_light_turn_on();
void hdmi_light_turn_off();
void gpio_init();

#endif