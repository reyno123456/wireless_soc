/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_sram.c
Description: The external HAL APIs to use the SRAM.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/21
History: 
        0.0.1    2016/12/21    The initial version of hal_sram.c
*****************************************************************************/

#include <stdint.h>
#include "sram.h"
#include "hal_sram.h"
#include "hal_nvic.h"
#include "interrupt.h"
#include "hal_usb_otg.h"
#include "debuglog.h"
#include "usbd_hid.h"


/**
* @brief  Config the Buffer in SRAM to Receive Video Data from SKY.
* @param  void
* @retval   void
* @note  
*/
void HAL_SRAM_ReceiveVideoConfig(void)
{
    SRAM_GROUND_ReceiveVideoConfig();

    /* register the irq handler */
    reg_IrqHandle(BB_SRAM_READY_IRQ_0_VECTOR_NUM, SRAM_Ready0IRQHandler, NULL);
    INTR_NVIC_SetIRQPriority(BB_SRAM_READY_IRQ_0_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_SRAM0,0));

    /* enable the SRAM_READY_0 IRQ */
    INTR_NVIC_EnableIRQ(BB_SRAM_READY_IRQ_0_VECTOR_NUM);

    /* register the irq handler */
    reg_IrqHandle(BB_SRAM_READY_IRQ_1_VECTOR_NUM, SRAM_Ready1IRQHandler, NULL);
    INTR_NVIC_SetIRQPriority(BB_SRAM_READY_IRQ_1_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_SRAM1,0));

    /* enable the SRAM_READY_1 IRQ */
    INTR_NVIC_EnableIRQ(BB_SRAM_READY_IRQ_1_VECTOR_NUM);
}


/**
* @brief  Enable Sky Video Bypass Function
* @param  e_sramVideoCh                                     indicate which video channel to use
* @retval   HAL_OK                                               reset buffer success
*               HAL_SRAM_ERR_CHANNEL_INVALID      the input channel number is not correct
* @note  
*/
HAL_RET_T HAL_SRAM_EnableSkyBypassVideo(ENUM_HAL_SRAM_VIDEO_CHANNEL e_sramVideoCh)
{
    if (e_sramVideoCh > HAL_SRAM_VIDEO_CHANNEL_1)
    {
        return HAL_SRAM_ERR_CHANNEL_INVALID;
    }

    SRAM_SKY_EnableBypassVideoConfig(e_sramVideoCh);

    return HAL_OK;
}


/**
* @brief  Disable Sky Video Bypass Function
* @param  e_sramVideoCh                                     indicate which video channel to use
* @retval   HAL_OK                                               reset buffer success
*               HAL_SRAM_ERR_CHANNEL_INVALID      the input channel number is not correct
* @note  
*/
HAL_RET_T HAL_SRAM_DisableSkyBypassVideo(ENUM_HAL_SRAM_VIDEO_CHANNEL e_sramVideoCh)
{
    if (e_sramVideoCh > HAL_SRAM_VIDEO_CHANNEL_1)
    {
        return HAL_SRAM_ERR_CHANNEL_INVALID;
    }

    SRAM_SKY_DisableBypassVideoConfig(e_sramVideoCh);

    return HAL_OK;
}


void HAL_SRAM_CheckChannelTimeout(void)
{
    SRAM_CheckTimeout();
}


void HAL_SRAM_ChannelConfig(ENUM_HAL_SRAM_CHANNEL_TYPE e_channelType,
                           ENUM_HAL_USB_PORT e_usbPort,
                           uint8_t u8_channel)
{
    uint8_t         u8_sramChannel;
    uint8_t         u8_usbEp;
    uint8_t         u8_usbPort;

    if (u8_channel > SRAM_CHANNEL_NUM)
    {
        dlog_error("u8_channel should not exceed 2");

        u8_sramChannel = 0;
    }
    else
    {
        u8_sramChannel = u8_channel;
    }

    u8_usbPort      = (uint8_t)e_usbPort;

    if (e_channelType == ENUM_HAL_SRAM_CHANNEL_TYPE_VIDEO0)
    {
        u8_usbEp    = HID_EPIN_VIDEO_ADDR;
    }
    else
    {
        /* video & audio use the same endpoint,
                  because BB has only two physical channel,
                  the first is occupied by Video0
                */
        u8_usbEp    = HID_EPIN_AUDIO_ADDR;
    }

    g_stChannelPortConfig[u8_sramChannel].u8_usbPort = u8_usbPort;
    g_stChannelPortConfig[u8_sramChannel].u8_usbEp = u8_usbEp;

    return;
}


uint8_t *HAL_SRAM_GetVideoBypassChannelBuff(ENUM_HAL_SRAM_VIDEO_CHANNEL e_channel)
{
    uint8_t          *channel_buff;

    if (e_channel == HAL_SRAM_VIDEO_CHANNEL_0)
    {
        channel_buff = (uint8_t *)VIDEO_BYPASS_CHANNEL_0_DEST_ADDR;
    }
    else
    {
        channel_buff = (uint8_t *)VIDEO_BYPASS_CHANNEL_1_DEST_ADDR;
    }

    return channel_buff;
}


#ifdef ARCAST
uint32_t HAL_SRAM_GetMp3BufferLength(void)
{
    return SRAM_GetMp3BufferLength();
}


uint32_t HAL_SRAM_GetMp3Data(uint32_t dataLen, uint8_t *dataBuff)
{
    return SRAM_GetMp3Data(dataLen, dataBuff);
}

void HAL_SRAM_ResetAudioRecv(void)
{
    if (sramReady0 == 1)
    {
        SRAM_Ready0Confirm();
    }
}


#endif




