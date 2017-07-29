/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_sram.h
Description: The external HAL APIs to use the SRAM.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/21
History: 
        0.0.1    2016/12/21    The initial version of hal_sram.h
*****************************************************************************/

#ifndef __HAL_SRAM_H__
#define __HAL_SRAM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "hal_ret_type.h"
#include "hal_usb_otg.h"


typedef enum
{
    HAL_SRAM_VIDEO_CHANNEL_0    = 0,
    HAL_SRAM_VIDEO_CHANNEL_1    = 1,
    HAL_SRAM_VIDEO_CHANNEL_NUM  = 2,
} ENUM_HAL_SRAM_VIDEO_CHANNEL;


typedef enum
{
    ENUM_HAL_SRAM_CHANNEL_TYPE_VIDEO0  = 0,
    ENUM_HAL_SRAM_CHANNEL_TYPE_VIDEO1  = 1,
    ENUM_HAL_SRAM_CHANNEL_TYPE_AUDIO   = 2,
} ENUM_HAL_SRAM_CHANNEL_TYPE;


/**
* @brief  Config the Buffer in SRAM to Receive Video Data from SKY.
* @param  void
* @retval   void
* @note  
*/
void HAL_SRAM_ReceiveVideoConfig(void);

/**
* @brief  Enable Sky Video Bypass Function
* @param  e_sramVideoCh                                     indicate which video channel to use
* @retval   HAL_OK                                               reset buffer success
*               HAL_SRAM_ERR_CHANNEL_INVALID      the input channel number is not correct
* @note  
*/
HAL_RET_T HAL_SRAM_EnableSkyBypassVideo(ENUM_HAL_SRAM_VIDEO_CHANNEL e_sramVideoCh);

/**
* @brief  Disable Sky Video Bypass Function
* @param  e_sramVideoCh                                     indicate which video channel to use
* @retval   HAL_OK                                               reset buffer success
*               HAL_SRAM_ERR_CHANNEL_INVALID      the input channel number is not correct
* @note  
*/
HAL_RET_T HAL_SRAM_DisableSkyBypassVideo(ENUM_HAL_SRAM_VIDEO_CHANNEL e_sramVideoCh);


/**
* @brief  open the video output to PC or PAD.
* @param  void
* @retval   void
* @note  
*/
void HAL_SRAM_OpenVideo(void);


/**
* @brief  close the video output to PC or PAD.
* @param  void
* @retval   void
* @note  
*/
void HAL_SRAM_CloseVideo(void);

void HAL_SRAM_CheckChannelTimeout(void);

/**
* @brief  config the BB channel for video or audio
* @param  void
* @retval   void
* @note  
*/
void HAL_SRAM_ChannelConfig(ENUM_HAL_SRAM_CHANNEL_TYPE e_channelType,
                           ENUM_HAL_USB_PORT e_usbPort,
                           uint8_t u8_channel);


uint8_t *HAL_SRAM_GetVideoBypassChannelBuff(ENUM_HAL_SRAM_VIDEO_CHANNEL e_channel);

#ifdef ARCAST
uint32_t HAL_SRAM_GetMp3BufferLength(void);
uint32_t HAL_SRAM_GetMp3Data(uint32_t dataLen, uint8_t *dataBuff);
void HAL_SRAM_ResetAudioRecv(void);

#endif

#ifdef __cplusplus
}
#endif

#endif


