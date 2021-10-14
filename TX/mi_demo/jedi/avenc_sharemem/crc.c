#include <unistd.h>
#include "crc.h"


uint32 crc_check(uint8* data, uint32 length)
{
	unsigned short crc_reg = 0xFFFF;
	
	while (length--)
	{
		crc_reg = (crc_reg >> 8) ^ crc16_table[(crc_reg ^ *data++) & 0xff];
		
	}
	
	return (uint32)(~crc_reg) & 0x0000FFFF;
}


uint16 sum_check(uint8 *data, uint32 length)
{
	//unsigned long sum;
	unsigned short sum;
	
	//printf("buf : 0x%x \n", buf);
	
	for (sum = 0; length > 0; length--)
	{
		sum += *data++;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	//printf("buf.usChecksum: 0x%x \n", ~sum);
	
	return (unsigned short)(~sum);
}

