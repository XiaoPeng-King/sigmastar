#ifndef __GPIO_H_
#define __GPIO_H_

void create_gpio(unsigned char gpio_num);
void set_gpio_state(unsigned char gpio_num, unsigned char state);
char get_gpio_value(unsigned char gpio_num);
void set_gpio_value(unsigned char gpio_num, char value);



void init_gpio();
char detect_sw_id();
void eeprom_off();
void eeprom_on();
char get_key_value();
void key_init();
void cec_enable();
void cec_disenable();

#endif