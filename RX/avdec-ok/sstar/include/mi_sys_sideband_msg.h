/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

  Unless otherwise stipulated in writing, any and all information contained
 herein regardless in any format shall remain the sole proprietary of
 Sigmastar Technology Corp. and be kept in strict confidence
 (��Sigmastar Confidential Information��) by the recipient.
 Any unauthorized act including without limitation unauthorized disclosure,
 copying, use, reproduction, sale, distribution, modification, disassembling,
 reverse engineering and compiling of the contents of Sigmastar Confidential
 Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
 rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#ifndef _MI_SYS_SIDEBAND_MSG_H_
#define _MI_SYS_SIDEBAND_MSG_H_

#define MI_SYS_BITS_MASK(bits)  ((1ULL<<(bits))-1ULL)

#define MI_SYS_FIELD_BITS_MASK(start_bit, bit_count)  (MI_SYS_BITS_MASK(bit_count)<<(start_bit))

#define MI_SYS_FILED_VALUE(value, start_bit, bit_count)  ((((unsigned long long)(value))&MI_SYS_BITS_MASK(bit_count))<<(start_bit))

#define MI_SYS_FILED_CLEAR_FIELD(sideband_data, start_bit, bit_count) ((sideband_data) &~MI_SYS_FIELD_BITS_MASK(start_bit, bit_count))
#define MI_SYS_GET_FILED_VALUE(sideband_data, start_bit, bit_count)  (((sideband_data)>>(start_bit))&MI_SYS_BITS_MASK(bit_count))

#define MI_SYS_SET_FILED_VALUE(sideband_data, start_bit, bit_count, value)  \
   do{ (sideband_data) =  (MI_SYS_FILED_CLEAR_FIELD(sideband_data, start_bit, bit_count) |MI_SYS_FILED_VALUE(value,  start_bit, bit_count)); } while(0)

#define MI_SYS_SIDEBAND_MSG_TYPE_START_BIT 61
#define MI_SYS_SIDEBAND_MSG_TYPE_BIT_CNT 3
#define MI_SYS_SIDEBAND_MSG_ACK_BIT_START 60
#define MI_SYS_SIDEBAND_MSG_ACK_BIT_CNT 1

#define MI_SYS_SET_SIDEBAND_MSG_TYPE(sideband_data) MI_SYS_FILED_VALUE(sideband_data, MI_SYS_SIDEBAND_MSG_TYPE_START_BIT, MI_SYS_SIDEBAND_MSG_TYPE_BIT_CNT)
#define MI_SYS_GET_SIDEBAND_MSG_TYPE(sideband_data) MI_SYS_GET_FILED_VALUE(sideband_data, MI_SYS_SIDEBAND_MSG_TYPE_START_BIT, MI_SYS_SIDEBAND_MSG_TYPE_BIT_CNT)
#define MI_SYS_SIDEBAND_MSG_ACKED(sideband_data) MI_SYS_GET_FILED_VALUE(sideband_data, MI_SYS_SIDEBAND_MSG_ACK_BIT_START, MI_SYS_SIDEBAND_MSG_ACK_BIT_CNT)
#define MI_SYS_ACK_SIDEBAND_MSG(sideband_data) MI_SYS_SET_FILED_VALUE(sideband_data, MI_SYS_SIDEBAND_MSG_ACK_BIT_START, MI_SYS_SIDEBAND_MSG_ACK_BIT_CNT, 1)

#define  MI_SYS_SIDEBAND_MSG_NULL 0

#define  MI_SYS_SIDEBAND_MSG_TYPE_NULL             0
#define  MI_SYS_SIDEBAND_MSG_TYPE_PREFER_CROP_RECT 1
#define  MI_SYS_SIDEBAND_MSG_TYPE_PREFER_ROTATE 2
#define  MI_SYS_SIDEBAND_MSG_TYPE_NEED_ISP_AF_INFO 3

#define  MI_PREFER_CROP_RECT_X_START_BIT 0
#define  MI_PREFER_CROP_RECT_X_BIT_CNT 13   // 8k
#define  MI_PREFER_CROP_RECT_Y_START_BIT (MI_PREFER_CROP_RECT_X_START_BIT+MI_PREFER_CROP_RECT_X_BIT_CNT)
#define  MI_PREFER_CROP_RECT_Y_BIT_CNT 12 //  4k
#define  MI_PREFER_CROP_RECT_W_START_BIT (MI_PREFER_CROP_RECT_Y_START_BIT+MI_PREFER_CROP_RECT_Y_BIT_CNT)
#define  MI_PREFER_CROP_RECT_W_BIT_CNT 13   // 8k
#define  MI_PREFER_CROP_RECT_H_START_BIT (MI_PREFER_CROP_RECT_W_START_BIT+MI_PREFER_CROP_RECT_W_BIT_CNT)
#define  MI_PREFER_CROP_RECT_H_BIT_CNT 12 //  4k
#define  MI_PREFER_CROP_PREFER_FMT_START_BIT (MI_PREFER_CROP_RECT_H_START_BIT+MI_PREFER_CROP_RECT_H_BIT_CNT)
#define  MI_PREFER_CROP_PREFER_FMT_BIT_CNT 8 // fmt

#if (MI_PREFER_CROP_RECT_H_START_BIT+MI_PREFER_CROP_RECT_H_BIT_CNT>MI_SYS_SIDEBAND_MSG_ACK_BIT_START)
#error "MI_SYS_SIDEBAND_MSG_TYPE_PREFER_CROP_RECT META DATA BITs overflow!!!"
#endif
#define MI_SYS_MAKE_SIDEBAND_PREFER_CROP_MSG(x, y, w, h,fmt)  (MI_SYS_FILED_VALUE(MI_SYS_SIDEBAND_MSG_TYPE_PREFER_CROP_RECT, MI_SYS_SIDEBAND_MSG_TYPE_START_BIT, MI_SYS_SIDEBAND_MSG_TYPE_BIT_CNT)  \
                                         | MI_SYS_FILED_VALUE(x, MI_PREFER_CROP_RECT_X_START_BIT, MI_PREFER_CROP_RECT_X_BIT_CNT) \
                                         | MI_SYS_FILED_VALUE(y, MI_PREFER_CROP_RECT_Y_START_BIT, MI_PREFER_CROP_RECT_Y_BIT_CNT) \
                                         | MI_SYS_FILED_VALUE(w, MI_PREFER_CROP_RECT_W_START_BIT, MI_PREFER_CROP_RECT_W_BIT_CNT) \
                                         | MI_SYS_FILED_VALUE(h, MI_PREFER_CROP_RECT_H_START_BIT, MI_PREFER_CROP_RECT_H_BIT_CNT) \
                                         | MI_SYS_FILED_VALUE(fmt, MI_PREFER_CROP_PREFER_FMT_START_BIT, MI_PREFER_CROP_PREFER_FMT_BIT_CNT)) \

#define MI_SYS_GET_PREFER_CROP_MSG_DAT(sideband_data, x, y, w, h, fmt)  \
     do{  \
         MI_SYS_BUG_ON(MI_SYS_GET_FILED_VALUE((sideband_data), MI_SYS_SIDEBAND_MSG_TYPE_START_BIT, MI_SYS_SIDEBAND_MSG_TYPE_BIT_CNT)!=MI_SYS_SIDEBAND_MSG_TYPE_PREFER_CROP_RECT);  \
         (x)= MI_SYS_GET_FILED_VALUE((sideband_data), MI_PREFER_CROP_RECT_X_START_BIT, MI_PREFER_CROP_RECT_X_BIT_CNT);  \
         (y)= MI_SYS_GET_FILED_VALUE((sideband_data), MI_PREFER_CROP_RECT_Y_START_BIT, MI_PREFER_CROP_RECT_Y_BIT_CNT);  \
         (w)= MI_SYS_GET_FILED_VALUE((sideband_data), MI_PREFER_CROP_RECT_W_START_BIT, MI_PREFER_CROP_RECT_W_BIT_CNT);  \
         (h)= MI_SYS_GET_FILED_VALUE((sideband_data), MI_PREFER_CROP_RECT_H_START_BIT, MI_PREFER_CROP_RECT_H_BIT_CNT);  \
         (fmt)= MI_SYS_GET_FILED_VALUE((sideband_data), MI_PREFER_CROP_PREFER_FMT_START_BIT, MI_PREFER_CROP_PREFER_FMT_BIT_CNT); }while(0)

#define MI_SYS_MAKE_SIDEBAND_PREFER_ROTATE_MSG (MI_SYS_FILED_VALUE(MI_SYS_SIDEBAND_MSG_TYPE_PREFER_ROTATE, MI_SYS_SIDEBAND_MSG_TYPE_START_BIT, MI_SYS_SIDEBAND_MSG_TYPE_BIT_CNT))

#endif
