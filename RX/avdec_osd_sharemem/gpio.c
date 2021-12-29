#include <stdlib.h>
#include <stdio.h>
#include "linux_common.h"

#define CEC_5V 			27

#define SWITCH_1		28
#define SWITCH_2		29
#define SWITCH_3		30
#define SWITCH_4		31
#define SWITCH_5		32
#define SWITCH_6		33
#define SWITCH_7		34

#define EEPROM_LOCK 	86

#define KEY				26

#define INPUT 1
#define OUTPUT 0

#define HIGH 1
#define LOW 0

static void create_gpio(unsigned char gpio_num)
{
	char cmd[50] = {};
	char detect[50] = {};

	sprintf(detect, "/sys/class/gpio/gpio%d", gpio_num);
	if (0 == IsExistent(detect))
	{
		printf("%s has been \n", detect);
		return 0;
	}

	sprintf(cmd, "echo %d > /sys/class/gpio/export", gpio_num);
	printf(cmd);
	system(cmd);
	return 0;
}

static void set_gpio_state(unsigned char gpio_num, unsigned char state)
{
	char cmd[128] = {};
	char path[64] = {};
	int ret = -1;

	if ((27>gpio_num)&&(33<gpio_num))
	{
		printf("the gpio number is error \n");
	}

	if (1 == state)
	{
		sprintf(path, "/sys/devices/gpiochip0/gpio/gpio%d/direction", gpio_num);
		if (0==IsExistent(path))
		{
			sprintf(cmd, "echo in > %s", path);
			system(cmd);
		}
	}
	else if (0 == state)
	{
		sprintf(path, "/sys/devices/gpiochip0/gpio/gpio%d/direction", gpio_num);
		if (0==IsExistent(path))
		{
			sprintf(cmd, "echo out > %s", path);
			system(cmd);
		}
	}
	else
	{
		printf("gpio state is error \n");
	}
}

static char get_gpio_value(unsigned char gpio_num)
{
	char value[8] = {};
	char path[64] = {};
	char ret = -1;
	FILE *pF = NULL;

	sprintf(path, "/sys/devices/gpiochip0/gpio/gpio%d/value", gpio_num);
	if (0==IsExistent(path))
	{
		if (NULL == pF)
		{
			pF = OpenFile(path, "r");
			ReadFile(value, 1, 1, pF);
			fclose(pF);
		}
		
		if (0 == strcmp("1", value))
		{
			return 1;
		}
		else if (0 == strcmp("0", value))
		{
			return 0;
		}
		else
		{
			printf("read gpio value error \n");
			return -1;
		}
	}
}

static void set_gpio_value(unsigned char gpio_num, char value)
{
	char path[64] = {};
	FILE *pF = NULL;

	if ((20>gpio_num)&&(90<gpio_num))
	{
		printf("the gpio number is error \n");
	}

	sprintf(path, "/sys/devices/gpiochip0/gpio/gpio%d/value", gpio_num);

	if (0==IsExistent(path))
	{
		pF = OpenFile(path, "w");
		if (1 == value)
		{
			WriteToFile("1", 1, 1, pF);
		}
		else if (0 == value)
		{
			WriteToFile("0", 1, 1, pF);
		}
		else
		{
			printf("write value error \n");
		}
		fclose(pF);
	}
	return ;
}

static void eeprom_lock_init()
{
	create_gpio(EEPROM_LOCK);
	set_gpio_state(EEPROM_LOCK, OUTPUT);
}

void eeprom_off()
{
	set_gpio_value(EEPROM_LOCK, HIGH);
	return ;
}

void eeprom_on()
{
	set_gpio_value(EEPROM_LOCK, LOW);
	return ;
}

//create gpio option dir
static void create_sw_gpio()
{
	create_gpio(SWITCH_1);
	create_gpio(SWITCH_2);
	create_gpio(SWITCH_3);
	create_gpio(SWITCH_4);
	create_gpio(SWITCH_5);
	create_gpio(SWITCH_6);
	create_gpio(SWITCH_7);

	return 0;
}

static void set_sw_input()
{
	set_gpio_state(SWITCH_1, INPUT);
	set_gpio_state(SWITCH_2, INPUT);
	set_gpio_state(SWITCH_3, INPUT);
	set_gpio_state(SWITCH_4, INPUT);
	set_gpio_state(SWITCH_5, INPUT);
	set_gpio_state(SWITCH_6, INPUT);
	set_gpio_state(SWITCH_7, INPUT);

	return 0;
}

char detect_sw_id()
{
	char id = 0;
	char flag = 0;

	flag = get_gpio_value(SWITCH_7);
	id = (id | (flag << 0));
	
	flag = get_gpio_value(SWITCH_6);
	id = (id | (flag << 1));

	flag = get_gpio_value(SWITCH_5);
	id = (id | (flag << 2));

	flag = get_gpio_value(SWITCH_4);
	id = (id | (flag << 3));

	flag = get_gpio_value(SWITCH_3);
	id = (id | (flag << 4));

	flag = get_gpio_value(SWITCH_2);
	id = (id | (flag << 5));

	flag = get_gpio_value(SWITCH_1);
	id = (id | (flag << 6));

	//printf("detect sw id : 0x%x \n", id);

	return id;
}

static void create_key()
{
	create_gpio(KEY);

	return;
}

static void set_key_input()
{
	set_gpio_state(KEY, INPUT);

	return;
}

char get_key_value()
{
	unsigned char value = 0;

	value = get_gpio_value(KEY);

	return value;
}

static void create_cec_5v()
{
	create_gpio(CEC_5V);
	return;
}

void cec_enable()
{
	create_gpio(CEC_5V);
	set_gpio_state(CEC_5V, OUTPUT);
	set_gpio_value(CEC_5V, HIGH);
}

void cec_disenable()
{
	create_gpio(CEC_5V);
	set_gpio_state(CEC_5V, OUTPUT);
	set_gpio_value(CEC_5V, LOW);
}

void init_gpio()
{
	create_sw_gpio();
	create_cec_5v();
	cec_enable();
	create_key();
	eeprom_lock_init();
	set_sw_input();
	set_key_input();
	detect_sw_id();
}




//

