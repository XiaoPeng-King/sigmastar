#ifndef _SETTING_H_
#define _SETTING_H_
//#include "../version.h"
#include "linked_list.h"
#define WEB_VERSION "TEST"

#define HD_VERSION "HW11.2"
#define SW_VERSION WEB_VERSION //version number is smaller than 20 byte

#define RTMP_ENABLE 1
#define RTMP_DISABLE 0
#define RTSP_ENABLE 1
#define RTSP_DISABLE 0
#define SDI_IN 2
#define HDMI_IN 1
#define NONE 0
#define WLAN_DHCP_ENABLE 1
#define WLAN_DHCP_DISABLE 0
#define INTERFACE_WLAN0 1
#define INTERFACE_ETH0 0

#define WPA 1
#define NO_EYP 0

#define DISABLE 0
#define ENABLE 1

typedef struct{
    unsigned char ucInputStatus; //0:none 1:HDMI 2:SDI
    unsigned short usWidth;
    unsigned short usHeight;
    unsigned short ucFrameRate;
    unsigned short usAudioSampleRate;
    char strHardwareVer[20];
    char strSoftwareVer[20];
}RUN_STATUS;

RUN_STATUS run_status;

typedef struct{
    char strEthIp[20];
    char strEthMask[20];
    char strEthGateway[20];
	char strEthMulticast[20];
}ETH_SETTING;
ETH_SETTING eth_setting;

typedef struct{
    unsigned short usEncRate;
    unsigned char ucInterval;
    unsigned char ucFrameRate;
    unsigned char ucIQP;
    unsigned char ucPQP;
}ENCODER_SETTING;
ENCODER_SETTING encoder_setting;

//wang
typedef struct
{
	unsigned char ucIpAddress[128]; 
	unsigned char ucMultiAddress[128]; 
	//unsigned int uuid[128]; 
    pNODE_S uuid[128];
} GROUP_PACK_S;
GROUP_PACK_S group_pack;

typedef struct
{
	char rxRename[128][40];
	char txRename[24][40];
} GROUP_RENAME_S;
GROUP_RENAME_S group_rename;

typedef struct
{
	char modeRename[10][20];
} MODE_RENAME_S;
MODE_RENAME_S mode_rename;

#endif




