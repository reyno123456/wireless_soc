/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_softi2s.c
Description: this module contains the helper fucntions necessary to control the general
             purpose softi2s block.softi2s use gpio to read i2s data.
             audio data buff limit 1M (AUDIO_DATA_END-AUDIO_DATA_START).
NOTE: this file don't use -O1 to complie
Author: Artosy Software Team
Version: 0.0.1
Date: 2017/02/21
History:
         0.0.1    2017/02/21    The initial version of hal_softi2s.c
*****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "cpu_info.h"
#include "pll_ctrl.h"
#include "systicks.h"
#include "debuglog.h"

#include "hal_ret_type.h"
#include "hal_nvic.h"
#include "hal_gpio.h"
#include "hal_softi2s.h"
#include "dma.h"
#include "hal_usb_device.h"
#include "hal_sram.h"
#include "memory_config.h"
#include "inter_core.h"

volatile uint16_t g_u16_audioDataArray[ADUIO_DATA_BUFF_LENGHT]={0};
volatile uint32_t g_u32_audioDataConut=0;
volatile uint32_t g_u32_audioDataReady=1;
volatile uint8_t  g_u8_audioDataOffset=0;
volatile uint32_t g_u32_audioDataAddr;
volatile uint32_t g_u32_audioLeftInterruptAddr=0;
volatile uint32_t g_u32_audioRightInterruptAddr=0;
volatile uint32_t g_u32_audioFlage=1;

static volatile uint32_t g_u32_dstAddress = AUDIO_DATA_START;

HAL_RET_T HAL_SOFTI2S_Init(STRU_HAL_SOFTI2S_INIT *st_i2sInit)
{

    volatile uint32_t *pu8_newAudioSampleRate=(uint32_t *)(SRAM_MODULE_SHARE_AUDIO_RATE);
    
    if (((st_i2sInit->e_audioLeftGpioNum)/32) == ((st_i2sInit->e_audioRightGpioNum)/32))
    {
        return HAL_SOFTI2S_ERR_INIT;
    }

    //SysTicks_UnInit();

    uint32_t i=0;
    for(i=16;i<98;i++)
    {
      HAL_NVIC_DisableIrq(i);
    }


    g_u8_audioDataOffset=(st_i2sInit->e_audioDataGpioNum%32)%8;
    g_u32_audioDataAddr= ((st_i2sInit->e_audioDataGpioNum%32)>>3)*0x04 + 0x50 + (st_i2sInit->e_audioDataGpioNum>>5)*0x40000 + 0x40400000;
    g_u32_audioLeftInterruptAddr = (st_i2sInit->e_audioLeftGpioNum>>5)*0x40000 + 0x4040004C;
    g_u32_audioRightInterruptAddr = (st_i2sInit->e_audioRightGpioNum>>5)*0x40000 + 0x4040004C;
    
    memset((uint8_t *)AUDIO_DATA_START,0,(AUDIO_DATA_END - AUDIO_DATA_START));

    HAL_GPIO_InPut(st_i2sInit->e_audioDataGpioNum);    
    //left
    HAL_NVIC_SetPriority(HAL_NVIC_GPIO_INTR_N0_VECTOR_NUM + (st_i2sInit->e_audioLeftGpioNum>>5),4,0);
    HAL_NVIC_SetPriority(HAL_NVIC_GPIO_INTR_N0_VECTOR_NUM + (st_i2sInit->e_audioRightGpioNum>>5),4,0);

    HAL_GPIO_RegisterInterrupt(st_i2sInit->e_audioLeftGpioNum, HAL_GPIO_EDGE_SENUMSITIVE, HAL_GPIO_ACTIVE_LOW, NULL);

    //right   
    HAL_GPIO_RegisterInterrupt(st_i2sInit->e_audioRightGpioNum, HAL_GPIO_EDGE_SENUMSITIVE, HAL_GPIO_ACTIVE_HIGH, NULL);
    dlog_info("i2s init %p %p %p %p\n",LeftAudio_48K,RightAudio_48K,LeftAudio_44p1K,RightAudio_44p1K);

    if (HAL_SOFTI2S_ENCODE_IEC_44100 == *pu8_newAudioSampleRate)
    {
        *((uint32_t *)(AUDIO_LEFT_INTERRUPT_ADDR)) = (uint32_t)&LeftAudio_44p1K;
        *((uint32_t *)(AUDIO_RIGHT_INTERRUPT_ADDR)) = (uint32_t)&RightAudio_44p1K;
    }
    else if (HAL_SOFTI2S_ENCODE_IEC_48000 == *pu8_newAudioSampleRate)
    {
        *((uint32_t *)(AUDIO_LEFT_INTERRUPT_ADDR)) = (uint32_t)&LeftAudio_48K;
        *((uint32_t *)(AUDIO_RIGHT_INTERRUPT_ADDR)) = (uint32_t)&RightAudio_48K;
    }

    return  HAL_OK;
}

extern uint8_t InterCore_GetMsg(uint32_t* msg_p, uint8_t* buf, uint32_t max_length);
void HAL_SOFTI2S_Funct(void)
{
    volatile uint32_t *pu32_newPcmDataFlagAddr=(uint32_t *)(SRAM_MODULE_SHARE_AUDIO_PCM);
    volatile uint32_t *pu8_newAudioSampleRate=(uint32_t *)(SRAM_MODULE_SHARE_AUDIO_RATE);
    uint32_t u8_audioSampleRateTmp=0xf;
    while(1)
    {
        if (0 == g_u32_audioDataReady)
        {                      
            #ifdef AUDIO_SDRAM
            //DMA_transfer((uint32_t)g_u16_audioDataArray+DTCM_CPU1_DMA_ADDR_OFFSET, g_u32_dstAddress, \
                        (ADUIO_DATA_BUFF_LENGHT*sizeof(uint16_t)), CHAN0, LINK_LIST_ITEM);
            memcpy((void *)g_u32_dstAddress,(void *)g_u16_audioDataArray,(ADUIO_DATA_BUFF_LENGHT*sizeof(uint16_t)));
            #else
            //DMA_transfer((uint32_t)g_u16_audioDataArray+DTCM_CPU1_DMA_ADDR_OFFSET, g_u32_dstAddress+DTCM_CPU0_DMA_ADDR_OFFSET, \
                        (ADUIO_DATA_BUFF_LENGHT*sizeof(uint16_t)), CHAN0, LINK_LIST_ITEM);
            memcpy((void *)(g_u32_dstAddress+DTCM_CPU0_DMA_ADDR_OFFSET),(void *)g_u16_audioDataArray,(ADUIO_DATA_BUFF_LENGHT*sizeof(uint16_t)));
            #endif            
            g_u32_audioDataReady=1;            
            g_u32_dstAddress+=(ADUIO_DATA_BUFF_LENGHT*sizeof(uint16_t));
            if (AUDIO_DATA_BUFF_SIZE == g_u32_dstAddress-AUDIO_DATA_START)
            {
                *pu32_newPcmDataFlagAddr = 1;
                //dlog_info("OK %x %d %p\n",g_u32_dstAddress,*pu32_newPcmDataFlagAddr,pu32_newPcmDataFlagAddr); 
            }
            if (AUDIO_DATA_END == g_u32_dstAddress)
            {
                g_u32_dstAddress = AUDIO_DATA_START;
                *pu32_newPcmDataFlagAddr = 2;
                //dlog_info("OK %x %d %p\n",g_u32_dstAddress,*pu32_newPcmDataFlagAddr,pu32_newPcmDataFlagAddr);                          
            } 
        }
        else if (u8_audioSampleRateTmp != *pu8_newAudioSampleRate)
        {
            if (0 == *pu8_newAudioSampleRate)
            {
                *((uint32_t *)(AUDIO_LEFT_INTERRUPT_ADDR)) = (uint32_t)&LeftAudio_44p1K;
                *((uint32_t *)(AUDIO_RIGHT_INTERRUPT_ADDR)) = (uint32_t)&RightAudio_44p1K;
            }
            else if (2 == *pu8_newAudioSampleRate)
            {
                *((uint32_t *)(AUDIO_LEFT_INTERRUPT_ADDR)) = (uint32_t)&LeftAudio_48K;
                *((uint32_t *)(AUDIO_RIGHT_INTERRUPT_ADDR)) = (uint32_t)&RightAudio_48K;
            }
            u8_audioSampleRateTmp = *pu8_newAudioSampleRate;
            //dlog_info("OK %x %x %p %p\n",u8_audioSampleRateTmp,*pu8_newAudioSampleRate,*((uint32_t *)(AUDIO_RIGHT_INTERRUPT_ADDR)),*((uint32_t *)(AUDIO_LEFT_INTERRUPT_ADDR)));             
        }
        else
        {
            uint32_t msg = 0; 
            uint8_t buf[INTER_CORE_MSG_SHARE_MEMORY_DATA_LENGTH];
            InterCore_GetMsg(&msg, buf,   sizeof(buf));
        }
        
    }
    
}
