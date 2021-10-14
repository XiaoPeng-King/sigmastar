#ifndef _SETTING_H_
#define _SETTING_H_
//#include "../version.h"

#define HD_VERSION "HW11.2"
#define SW_VERSION WEB_VERSION //version number is smaller than 20 byte

#define SW_VERSION "CEC_TEST"
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

typedef struct
{
	char cec_code_on[10];
    char cec_code_off[10];
    char cec_state; //0:normel 1:on 2:off
} CEC_CONTROL_S;

#endif
