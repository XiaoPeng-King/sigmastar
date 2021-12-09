#ifndef __HDMI_INFO_H_
#define __HDMI_INFO_H_

typedef struct _H264_APPEND_INFO
{
        //audio
        unsigned int fs;
        unsigned int audio_bits;
        unsigned int chns;
        //unsigned int audio_handle;
        //video
        unsigned int width;
        unsigned int height;
        unsigned int src_width;
        unsigned int src_height;
        unsigned char frameRate;
        unsigned char refreshRate;
        unsigned char interlace;
}H264_APPEND_INFO_s;
H264_APPEND_INFO_s h264_append_info;

int init_info(void);
char check_hdmi_signal(H264_APPEND_INFO_s *info);
char check_hdmi(void);

#endif