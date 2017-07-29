/****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_softi2s.h
Description: this module contains the helper fucntions necessary to control the general
             purpose softi2s block.softi2s use gpio to read i2s data.
             audio data buff limit 1M (AUDIO_SDRAM_END-AUDIO_SDRAM_START).
Author: Artosy Software Team
Version: 0.0.1
Date: 2017/02/21
History:
         0.0.1    2016/12/19    The initial version of hal_softi2s.h
*****************************************************************************/

#ifndef __HAL_SOFTI2S_H__
#define __HAL_SOFTI2S_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "hal_gpio.h"

#define AUDIO_LEFT_INTERRUPT_ADDR        (67*4)
#define AUDIO_RIGHT_INTERRUPT_ADDR       (68*4)

#define ADUIO_DATA_BUFF_LENGHT           (4608/sizeof(uint16_t)) 

typedef enum
{
    HAL_SOFTI2S_ENCODE_IEC_44100 = 0,
    HAL_SOFTI2S_ENCODE_IEC_NOT_INDICATED,
    HAL_SOFTI2S_ENCODE_IEC_48000,
    HAL_SOFTI2S_ENCODE_IEC_32000,
    HAL_SOFTI2S_ENCODE_IEC_24000 = 6
} ENUM_HAL_I2S_IEC_SAMPLERATE;


//#define AUDIO_SDRAM                     (1)


#ifdef  AUDIO_SDRAM
#define AUDIO_DATA_BUFF_COUNT           (1)         
#define AUDIO_DATA_START                (0x81F00000)
#define AUDIO_DATA_BUFF_SIZE            (ADUIO_DATA_BUFF_LENGHT*AUDIO_DATA_BUFF_COUNT*2)    
#define AUDIO_DATA_TOTILE_BUFF_SIZE     (AUDIO_DATA_BUFF_SIZE*2)
#define AUDIO_DATA_END                  (AUDIO_DATA_START+AUDIO_DATA_TOTILE_BUFF_SIZE)
#else
#define AUDIO_DATA_BUFF_COUNT           (1) 
#define AUDIO_DATA_START                (0x20078000)
#define AUDIO_DATA_BUFF_SIZE            (ADUIO_DATA_BUFF_LENGHT*AUDIO_DATA_BUFF_COUNT*2)    
#define AUDIO_DATA_TOTILE_BUFF_SIZE     (AUDIO_DATA_BUFF_SIZE*2)
#define AUDIO_DATA_END                  (AUDIO_DATA_START+AUDIO_DATA_TOTILE_BUFF_SIZE)
#endif



typedef struct
{
    uint32_t u32_audioDataAddress;
    ENUM_HAL_GPIO_NUM e_audioLeftGpioNum;
    ENUM_HAL_GPIO_NUM e_audioRightGpioNum;
    ENUM_HAL_GPIO_NUM e_audioDataGpioNum;    
} STRU_HAL_SOFTI2S_INIT;

/**
* @brief    soft i2s initialization
* @param    STRU_HAL_SOFTI2S_INIT: I2S initialization.
* @retval   HAL_OK    means the initialization is well done.
* @retval   HAL_SOFTI2S_ERR_INIT    means interrput gpio comflict.
* @note     none
*/
HAL_RET_T HAL_SOFTI2S_Init(STRU_HAL_SOFTI2S_INIT *st_i2sInit);

/**
* @brief    soft i2s run function
* @param    none
* @retval   none
* @note     none
*/
void HAL_SOFTI2S_Funct(void);
void LeftAudio_44p1K(void);
void RightAudio_44p1K(void);
void LeftAudio_48K(void);
void RightAudio_48K(void);


#ifdef __cplusplus
}
#endif

#endif /*__HAL_SOFTI2S_H__END*/
