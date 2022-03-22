#ifndef _MAIN_H_
#define _MAIN_H_
#include <stdbool.h>

#define TRUE	1
#define FALSE	0

typedef enum {
	EN_NO_HDMI = 0,
	EN_NO_TX,
} SIGNAL_ABNORMAL_e;

typedef enum {
	EN_IP_CONFLICT = 0,
	EN_INFO_DIAPLAY,
	EN_CLOSE_DISPLAY,
} SIGNAL_NORMAL_e;

typedef enum {
	EN_ABNORMAL = 0,
	EN_NORMAL,
} SIGNAL_STATE_e;

typedef struct {
	SIGNAL_STATE_e signal_state;
	SIGNAL_ABNORMAL_e signal_abnormal;
	SIGNAL_NORMAL_e signal_normal;
} DISPLAY_STATE_s;

typedef struct {
	DISPLAY_STATE_s display_state;

	char multicast[20];
	char serverip[20];
	bool display_flag;
	bool multicast_change_flag; //
	bool e2prom; //
	bool web_flag; //
	bool info_display; //
	bool ip_conflict_flag; //
} SYSTEM_ATTR_s;


#endif 