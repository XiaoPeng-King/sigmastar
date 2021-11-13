#ifndef _EEPROM_H_
#define _EEPROM_H_

#include "sharemem.h"

#define ERROR -1
#define SUCCEED 0

typedef short int int16_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;

typedef unsigned char uint8_t;

typedef struct address_to_ctrl_byte
{
	uint8_t ctrl_byte;
	uint8_t word_addr;
}addr_ctrl_byte_struct;

int init_eeprom();
int deinit_eeprom();

addr_ctrl_byte_struct get_eightbytes(uint16_t address_in_chip);

int i2c_page_write(uint8_t ctrl_byte,uint8_t word_addr,uint8_t *source_data_addr,uint8_t data_len);
int i2c_sequential_read(uint8_t ctrl_byte,uint8_t word_addr,uint16_t data_len,uint8_t *data_addr_in_master_mem);
int i2c_write_within_block(uint8_t ctrl_byte,uint8_t word_addr,uint8_t *source_data_addr,uint16_t data_len);
int i2c_write_within_chip(uint16_t address_in_chip,uint8_t *source_data_addr,uint16_t data_len);
int i2c_read_within_chip(uint16_t address_in_chip,uint8_t *data_addr_in_master_mem,uint16_t data_len);


//void test(void);


#endif



