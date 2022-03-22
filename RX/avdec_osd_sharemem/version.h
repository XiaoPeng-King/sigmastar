#ifndef VERSION_H_
#define VERSION_H_

//#define RM_CONFIG_FILE ("rm /customer/configs/config.conf")
//#define CONFIG_FILE1 ("/customer/configs/modeName.conf")
//#define CONFIG_FILE ("/customer/configs/config.conf")

#define RM_CONFIG_FILE ("rm ./config.conf")
#define CONFIG_FILE1 ("./modeName.conf")
#define CONFIG_FILE ("./config.conf")

#define CONFIG_EEPROM

#if 1
#define SW_KEY //拨码开关
#else
#define DIGIT_LED	//数码管
#endif

#endif
