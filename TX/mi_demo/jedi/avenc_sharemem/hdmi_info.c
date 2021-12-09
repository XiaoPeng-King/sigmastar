#include "mi_sensor.h"
#include "hdmi_info.h"


typedef enum
{
    EN_CUST_CMD_GET_STATE,
    EN_CUST_CMD_GET_AUDIO_INFO,
    EN_CUST_CMD_CONFIG_I2S,
    EN_CUST_CMD_MAX
}SS_HdmiConv_UsrCmd_e;


typedef enum
{
    EN_SIGNAL_NO_CONNECTION,
    EN_SIGNAL_CONNECTED,
    EN_SIGNAL_LOCK
}SS_HdmiConv_SignalInfo_e;


static int update_HDMI_info(H264_APPEND_INFO_s *info)
{
	int ret = -1;

    SS_HdmiConv_SignalInfo_e enTcState;
    MI_SNR_PlaneInfo_t stSnrPlane0Info;
    memset(&stSnrPlane0Info, 0x0, sizeof(MI_SNR_PlaneInfo_t)); 
    MI_U16 u16SnrWidth;
	MI_U16 u16SnrHeight;
	
	MI_SNR_CustFunction((MI_SNR_PAD_ID_e)0, EN_CUST_CMD_GET_STATE, sizeof(SS_HdmiConv_UsrCmd_e), (void *)&enTcState, E_MI_SNR_CUSTDATA_TO_USER);//获取HDMI视频信号状态
	
	//printf("enTcState : %d \n", enTcState);

	if (enTcState == EN_SIGNAL_LOCK)//如果信号稳定
	{
		MI_SNR_GetPlaneInfo((MI_SNR_PAD_ID_e)0, 0, &stSnrPlane0Info);//获取分辨率等信息
		u16SnrWidth = stSnrPlane0Info.stCapRect.u16Width;//从HDMI获取的分辨率宽
		u16SnrHeight = stSnrPlane0Info.stCapRect.u16Height;//从HDMI获取的分辨率高
		//printf("\n\nget enCurState %d u16SnrWidth %d, u16SnrHeight %d\n\n", enTcState,u16SnrWidth, u16SnrHeight);
		
		if ((u16SnrHeight==1080) && (u16SnrWidth==1920))
		{
			info->src_height = 1088;
			info->src_width = 1920;
			info->width = 1920;
			info->height = 1080;
			ret = 1;
		}
		else if ((u16SnrHeight==720) && (u16SnrWidth==1280))
		{
			info->src_height = 720;
			info->src_width = 1280;
			info->width = 1280;
			info->height = 720;
			ret = 1;
		}
		else
		{
			printf("can't support the solution ! \n\n");
			ret = 2;
		}
	}

	return ret;
}

int init_info(void)
{
	h264_append_info.src_height=720;//1088;
	h264_append_info.src_width=1280;//1920;
	h264_append_info.height=720;//1080;
	h264_append_info.width=1280;//1920;
	h264_append_info.audio_bits=16;
	h264_append_info.fs=44100;
	h264_append_info.chns=2;
	h264_append_info.frameRate=30;

	h264_append_info.interlace=0;
	h264_append_info.refreshRate=60;

    //printf("h264_append)info.fs:%d\n",h264_append_info.fs);
    //printf("h264_append_info.refreshRate: %d\n",h264_append_info.refreshRate);
}

char check_hdmi_signal(H264_APPEND_INFO_s *info)
{
	SS_HdmiConv_SignalInfo_e enTcState;
    MI_SNR_PlaneInfo_t stSnrPlane0Info;
    memset(&stSnrPlane0Info, 0x0, sizeof(MI_SNR_PlaneInfo_t)); 
    MI_U16 u16SnrWidth;
	MI_U16 u16SnrHeight;
	MI_SNR_CustFunction((MI_SNR_PAD_ID_e)0, EN_CUST_CMD_GET_STATE, sizeof(SS_HdmiConv_UsrCmd_e), (void *)&enTcState, E_MI_SNR_CUSTDATA_TO_USER);//获取HDMI视频信号状态
	
	//printf("enTcState : %d \n", enTcState);

	if (enTcState == EN_SIGNAL_LOCK)//如果信号稳定
	{
		MI_SNR_GetPlaneInfo((MI_SNR_PAD_ID_e)0, 0, &stSnrPlane0Info);//获取分辨率等信息
		u16SnrWidth = stSnrPlane0Info.stCapRect.u16Width;//从HDMI获取的分辨率宽
		u16SnrHeight = stSnrPlane0Info.stCapRect.u16Height;//从HDMI获取的分辨率高
		
		if ((u16SnrHeight != info->height) && (u16SnrWidth != info->width))
		{
			printf("\n\n u16SnrWidth %d, u16SnrHeight %d\n\n",u16SnrWidth, u16SnrHeight);
			printf("\n\n info->width %d, info->height %d\n\n",info->width, info->height);

			printf("HDMI resolution has changed !");
			update_HDMI_info(info);
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		sleep(1);
		printf("can't get video information from HDMI port !\n\n");
		printf("please check HDMI input statement.\n\n");
		return -1;
	}
}

char check_hdmi(void)
{
	SS_HdmiConv_SignalInfo_e enTcState;
    MI_SNR_PlaneInfo_t stSnrPlane0Info;
    memset(&stSnrPlane0Info, 0x0, sizeof(MI_SNR_PlaneInfo_t)); 
    MI_U16 u16SnrWidth;
	MI_U16 u16SnrHeight;
	MI_SNR_CustFunction((MI_SNR_PAD_ID_e)0, EN_CUST_CMD_GET_STATE, sizeof(SS_HdmiConv_UsrCmd_e), (void *)&enTcState, E_MI_SNR_CUSTDATA_TO_USER);//获取HDMI视频信号状态
	
	//printf("enTcState : %d \n", enTcState);

	if (enTcState == EN_SIGNAL_LOCK)//如果信号稳定
	{
		MI_SNR_GetPlaneInfo((MI_SNR_PAD_ID_e)0, 0, &stSnrPlane0Info);//获取分辨率等信息
		u16SnrWidth = stSnrPlane0Info.stCapRect.u16Width;//从HDMI获取的分辨率宽
		u16SnrHeight = stSnrPlane0Info.stCapRect.u16Height;//从HDMI获取的分辨率高
		//printf("\n\nget enCurState %d u16SnrWidth %d, u16SnrHeight %d\n\n", enTcState,u16SnrWidth, u16SnrHeight);
		return 0;
	}
	else
	{
		sleep(1);
		printf("can't get video information from HDMI port !\n\n");
		printf("please check HDMI input statement.\n\n");
		return -1;
	}
}