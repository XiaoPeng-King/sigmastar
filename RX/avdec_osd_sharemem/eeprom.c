#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>

#include "gpio.h"
#include "crc.h"
#include "iic.h"
#include "eeprom.h"

#define FILE_NAME "/dev/i2c-0"

/******************24C08 address space***********************/
//deivce	A2 pin	block	r/w		address
//1010 		1 		00 		1/0 	0-255
//1010 		1 		01 		1/0 	256-512
//1010 		1 		10 		1/0 	513-768
//1010 		1	 	11 		1/0 	769-1024
/***********************************************************/

/**
 * @brief 
 * Function : get_eigenbytes 
 * Description : 
 * Calls : 
 * Called By : 
 * Input : 
 *          
 * Output : 
 *          
 * Return : 
 *          
 * Other : NULL 
 * 
 */
addr_ctrl_byte_struct get_eigenbytes(uint16_t address_in_chip)
{
	addr_ctrl_byte_struct cbs;
	if((address_in_chip<0x00) || (address_in_chip>0x3FF))
	{
		printf("Cross-border error! The range of address_in_chip is 0x000-0x3FF.\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		if((address_in_chip>=0x00) && (address_in_chip<0x100)) //256
		{
			//cbs.ctrl_byte  = 0xA0; //1010 0000
			//cbs.ctrl_byte  = 0xA8; //1010 1000
			cbs.ctrl_byte  = 0x54; //0101 0100
			cbs.word_addr  = address_in_chip;
		}
		else if((address_in_chip>=0x100) && (address_in_chip<0x200)) //512
		{
			//cbs.ctrl_byte  = 0xA2; //1010 0010
			//cbs.ctrl_byte  = 0xAA; //1010 1010
			cbs.ctrl_byte  = 0x55; //0101 0101
			cbs.word_addr  = address_in_chip%0x100;
		}
		else if((address_in_chip>=0x200) && (address_in_chip<0x300)) //768
		{
			//cbs.ctrl_byte  = 0xA4; //1010 0100
			//cbs.ctrl_byte  = 0xAC; //1010 1100
			cbs.ctrl_byte  = 0x56; //0101 0110
			cbs.word_addr  = address_in_chip%0x200;
		}
		else
		{
			//cbs.ctrl_byte  = 0xA6; //1010 0110
			//cbs.ctrl_byte  = 0xAE; //1010 1110
			cbs.ctrl_byte  = 0x57; //0101 0111
			cbs.word_addr  = address_in_chip%0x300; 
		}
	}
	return cbs;
}


static int iic_fd = -1;

int init_eeprom()
{
	eeprom_on();
	iic_fd = open(FILE_NAME ,O_RDWR);
	if(iic_fd < 0)
	{
		perror("open /dev/i2c-1 failed\n");
		return -1;
	}
	printf("iic fd = %d \n", iic_fd);

	return 0;
}

int deinit_eeprom()
{
	if (iic_fd > 0)
		close(iic_fd);
		eeprom_off();
	iic_fd = -1;
	return 0;
}


// write bytes to e2prom (page write)
int i2c_page_write(uint8_t ctrl_byte,uint8_t word_addr,uint8_t *source_data_addr,uint8_t data_len)
{
	int ret = 0;
	uint8_t i;
	if (data_len<=0)
	{
		printf("i2c_page_write: data_len should be a positive number.\n");
		ret = -1;
	}
	else
	{
		i2c_bytes_write(iic_fd, ctrl_byte, word_addr, data_len, source_data_addr);
	}
	printf("i2c_page_write finished.\n");
	return ret;
}

// read sequential bytes from eeprom 
// it is the ADDRESS that is transferred, but not DATA!
//uint32_t i2c_sequential_read(uint8_t ctrl_byte,uint8_t word_addr,uint16_t data_num)
int i2c_sequential_read(uint8_t ctrl_byte,uint8_t word_addr,uint16_t data_len,uint8_t *data_addr_in_master_mem)
{
	int ret = 0;
	uint16_t i=0;
	
	if(data_len<=0)
	{
		printf("i2c_sequential_read: data_len should be a positive number.\n");
		return -1;
	}
	else
	{
		printf("---------test point 1---------\n");
		ret = i2c_bytes_read(iic_fd, ctrl_byte, word_addr, data_len, data_addr_in_master_mem);
	}
	printf("ctrl_byte: %x, word_addr: %x, data_len: %d, data: %s \n", ctrl_byte, word_addr, data_len, *data_addr_in_master_mem);
	
	printf("i2c_sequential_read finished.\n");
	return ret;
}

// memory_write within one single block
int i2c_write_within_block(uint8_t ctrl_byte,uint8_t word_addr,uint8_t *source_data_addr,uint16_t data_len)
{
	int ret = 0;
	uint8_t page_size = 0x10;
	uint16_t bolck_size = 0x100;
	uint8_t extra_page = 0;
	uint8_t i = 0;
	uint8_t page_offset = word_addr % page_size;
	uint8_t len_left = page_size - page_offset;
	
	// beyond the scope of the current block
	if ((word_addr + data_len) > bolck_size)
	{
		printf("(B1)i2c_write_within_block:beyond the scope of the current block, JUST RETURN.");
		ret = -1;
	}
	else// within the current block
	{
		if(data_len <= len_left)
		{
			printf("(B2)i2c_write_within_block:within the current page.\n");
			ret = i2c_page_write(ctrl_byte,word_addr,source_data_addr,data_len); // pointer as function parameter?
			//while(i2c_ack_check(ctrl_byte));
		}
		else
		{
			//printf("(B3)i2c_write_within_block:within the current block but beyond the current page.\n");
			if( (data_len - len_left)%page_size != 0 )
				extra_page = floor( (data_len - len_left)/(float)page_size+1 );
			else
				extra_page = (data_len - len_left)/(float)page_size;
			
			//printf("extra_page = %d.\n data_len = %d.\n len_left = %d.\n",extra_page,data_len,len_left);
			// first, write the current page
			ret = i2c_page_write(ctrl_byte,word_addr,source_data_addr,len_left);
			//while(i2c_ack_check(ctrl_byte));
			// then, write the following complete page except the last maybe-incomplete page
			for (i=1;i<extra_page;i++)
			{
				ret = i2c_page_write(ctrl_byte,(word_addr+len_left+(i-1)*page_size),(source_data_addr+len_left+(i-1)*page_size),page_size);
				//while(i2c_ack_check(ctrl_byte));
			}
			// finally, write the last maybe-incomplete page
			ret = i2c_page_write(ctrl_byte,(word_addr+len_left+(extra_page-1)*page_size),(source_data_addr+len_left+(extra_page-1)*page_size),
			(data_len-len_left-(extra_page-1)*page_size));
			//while(i2c_ack_check(ctrl_byte));
		}
	}
	printf("i2c_write_within_block finished.\n");
	return ret;
}


/******************** the following are 2 highest API:Write/Read in chip********************/

// memory_write within one 24c08 Chip 8kbit = 1kbyte 
int i2c_write_within_chip(uint16_t address_in_chip,uint8_t *source_data_addr,uint16_t data_len)
{
	int ret = 0;
	uint16_t block_size = 0x100;
	uint8_t extra_block = 0;
	uint8_t i = 0;
	
	addr_ctrl_byte_struct Scb;
	
	Scb = get_eigenbytes(address_in_chip);

	uint8_t ctrl_byte = Scb.ctrl_byte;
	//printf("%x \n", Scb.ctrl_byte);
	uint8_t word_addr = Scb.word_addr;
	//printf("%x \n", Scb.word_addr);
	uint8_t left_block_num = 4 - ((ctrl_byte & 0x06) >> 1);
	uint16_t total_mem_left = 1024-256*(4-left_block_num)-word_addr;
	uint16_t current_block_mem_left = block_size-word_addr;
	
	// do not beyond current block
	if ((word_addr+data_len) <= block_size)
	{
		printf("(C1)i2c_write_within_chip:do not beyond current block.\n");
		ret = i2c_write_within_block(ctrl_byte,word_addr,source_data_addr,data_len);
	}
	// the chip itself is not large enough
	// just write the chip full and abandon the left part
	else if (data_len > total_mem_left)
	{	
		printf("(C2)i2c_write_within_chip:the chip itself is not large enough.\n");
		data_len = total_mem_left;
		
		if( (data_len-current_block_mem_left)%block_size != 0 )
			extra_block = floor((data_len-current_block_mem_left)/(float)block_size + 1);
		else
			extra_block = (data_len-current_block_mem_left)/(float)block_size;
		
		// first, write the current block
		ret = i2c_write_within_block(ctrl_byte,word_addr,source_data_addr,current_block_mem_left);
		// just write the chip full and abandon the left part
		for (i=1;i <= extra_block;i++)
		{
			ret = i2c_write_within_block(ctrl_byte+0x02*i,0x00,source_data_addr+current_block_mem_left+(i-1)*block_size,block_size);
		}
	}
	// occupy more than one block of memory but not beyond chip's memory range
	else
	{
		printf("(C3)i2c_write_within_chip:occupy more than one block of memory but not beyond chip's memory range.\n");
		
		// judge whether extra_block is intergals or float
		if( (data_len-current_block_mem_left)%block_size != 0 )
			extra_block = floor((data_len-current_block_mem_left)/(float)block_size + 1);
		else
			extra_block = (data_len-current_block_mem_left)/(float)block_size;
		
		printf("extra_block = %d.\n",extra_block);
		// first, write the current block
		ret = i2c_write_within_block(ctrl_byte,word_addr,source_data_addr,current_block_mem_left);
		// then, write the following complete block except the last maybe-incomplete block
		for (i=1;i<extra_block;i++)
		{
			ret = i2c_write_within_block(ctrl_byte+0x02*i,0x00,source_data_addr+current_block_mem_left+(i-1)*block_size,block_size);
		}
		// finally, write the last maybe-incomplete block
		ret = i2c_write_within_block(ctrl_byte+0x02*extra_block,0x00,
			source_data_addr+current_block_mem_left+(extra_block-1)*block_size,
			data_len-current_block_mem_left-(extra_block-1)*block_size);
	}
	printf("i2c_write_within_chip finished.\n");
}

int i2c_read_within_chip(uint16_t address_in_chip,uint8_t *data_addr_in_master_mem,uint16_t data_len)
{
	int ret = 0;
	uint16_t block_size = 0x100; //256
	uint8_t extra_block = 0;
	uint8_t i = 0;
	
	addr_ctrl_byte_struct Scb;
	Scb = get_eigenbytes(address_in_chip);
	printf("Scb : %x \n", Scb);
	uint8_t ctrl_byte = Scb.ctrl_byte;
	uint8_t word_addr = Scb.word_addr;
	uint8_t left_block_num = 4 - ((ctrl_byte & 0x06) >> 1);
	uint16_t total_mem_left = 1024-256*(4-left_block_num)-word_addr;
	uint16_t current_block_mem_left = block_size-word_addr;
	
	// do not beyond current block
	if ((word_addr+data_len) <= block_size)
	{
		printf("(C1)i2c_read_within_chip:do not beyond current block.\n");
		ret = i2c_sequential_read(ctrl_byte,word_addr,data_len,data_addr_in_master_mem);
	}
	// the chip itself is not large enough
	// just read the chip full and abandon the left part
	else if (data_len > total_mem_left)
	{
		printf("(C2)i2c_read_within_chip:the chip itself is not large enough.\n");
		data_len = total_mem_left;
		
		if( (data_len-current_block_mem_left)%block_size != 0 )
			extra_block = floor((data_len-current_block_mem_left)/(float)block_size + 1);
		else
			extra_block = (data_len-current_block_mem_left)/(float)block_size;
		
		printf("data_len=%d\n current_block_mem_left=%d\n extra_block=%d\n",data_len,current_block_mem_left,extra_block);
		
		// first, read the current block
		ret = i2c_sequential_read(ctrl_byte,word_addr,current_block_mem_left,data_addr_in_master_mem);
		// then, read the following complete block till the end
		for (i = 1;i <= extra_block;i++)
		{
			ret = i2c_sequential_read(ctrl_byte+0x02*i,0x00,block_size,(data_addr_in_master_mem+current_block_mem_left+(i-1)*block_size));
		}
	}
	// occupy more than one block of memory but not beyond chip's memory range
	else
	{
		printf("(C3)i2c_read_within_chip:occupy more than one block of memory but not beyond chip's memory range.\n");
		
		if( (data_len-current_block_mem_left)%block_size != 0 )
			extra_block = floor((data_len-current_block_mem_left)/(float)block_size + 1);
		else
			extra_block = (data_len-current_block_mem_left)/(float)block_size;
		
		printf("data_len=%d\n current_block_mem_left=%d\n extra_block=%d\n",data_len,current_block_mem_left,extra_block);
		// first, read the current block
		ret = i2c_sequential_read(ctrl_byte,word_addr,current_block_mem_left,data_addr_in_master_mem);
		// then, read the following complete block till the end
		for (i = 1;i < extra_block;i++)
		{
			ret = i2c_sequential_read(ctrl_byte+0x02*i,0x00,block_size,(data_addr_in_master_mem+current_block_mem_left+(i-1)*block_size));
		}
		// finally, write the last maybe-incomplete block
		ret = i2c_sequential_read(ctrl_byte+0x02*extra_block,0x00,data_len-current_block_mem_left-(extra_block-1)*block_size,
		(data_addr_in_master_mem+current_block_mem_left+(extra_block-1)*block_size));
	}
	printf("i2c_read_within_chip finished.\n");
	return ret;
}

#if 0
void test(void)
{

	char rbuffer[sizeof(SHARE_MEM)] = {0};
	char w = 'A';
	char r;
	int i = 0;
	//i2c_gpio_init();
	sleep(1);
	int checksum = 0;
	//i2c_init();
	SHARE_MEM *pRead = malloc(sizeof(SHARE_MEM) * sizeof(char));
	while (1)
	{
		#if 1
		sleep(1);
		//i2c_sequential_read(0xA8, 0x00, sizeof(SHARE_MEM), pRead);
		printf("---------- eeprom read start ----------\n");
		printf("buffer size : %d \n", sizeof(SHARE_MEM));
		//i2c_read_within_chip(0x00, pRead, sizeof(SHARE_MEM));
		
		i2c_read_within_chip(0x00, pRead, sizeof(SHARE_MEM));
		printf("---------- eeprom read finish ----------\n");
		if (checksum == sum_check(pRead, sizeof(SHARE_MEM)))
		{
			printf("uuid: %d \n", pRead->uuid);
			printf("ip : %s \n", pRead->sm_eth_setting.strEthIp);
			printf("gateway : %s \n", pRead->sm_eth_setting.strEthGateway);
			printf("multicast : %s \n", pRead->sm_eth_setting.strEthMulticast);
		}
		else
		{
			printf("uuid: %d \n", pRead->uuid);
			printf("ip : %s \n", pRead->sm_eth_setting.strEthIp);
			printf("gateway : %s \n", pRead->sm_eth_setting.strEthGateway);
			printf("multicast : %s \n", pRead->sm_eth_setting.strEthMulticast);
			//printf("rbuffer : %s \n", rbuffer);
			printf("e2prom checksum error \n");
		}

		checksum = sum_check(share_mem, sizeof(SHARE_MEM));

		sleep(1);
		

		
		//while (1)
			sleep(1);
		#endif
	}
	//free(pRead);

	return 0;
}
#endif



