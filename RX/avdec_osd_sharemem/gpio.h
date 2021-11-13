#ifndef __GPIO_H_
#define __GPIO_H_

void init_gpio();
char detect_sw_id();
void eeprom_off();
void eeprom_on();
char get_key_value();

#endif