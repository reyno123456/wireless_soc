/*****************************************************************************
 * Copyright: 2016-2020, Artosyn. Co., Ltd.
 * File name: can.h
 * Description: mipi drive function declaration
 * Author: Artosyn FW
 * Version: V0.01 
 * Date: 2017.01.22
 * History: 
 * 2017.01.22 the first edition
 * *****************************************************************************/

#ifndef __MIPI_H__
#define __MIPI_H__


#include <stdint.h>



#define MIPI_BASE_ADDR       (0xA0050000)
#define ENCODER_BASE_ADDR    (0xA0010000)

typedef struct
{
    uint32_t u32_regAddr;
    uint32_t u32_val;
} STRU_REG_VALUE;


typedef struct
{
    uint8_t u8_mipiToEncoderCh;
    uint8_t u8_mipiFrameRate;
    uint16_t u16_mipiWidth;
    uint16_t u16_mipiHight;
} STRU_MIPI_INFO;


/**
* @brief    
* @param     
* @retval  
* @note    
*/
int32_t MIPI_Init(uint8_t u8_toEncoderCh, 
                  uint16_t u16_width,
                  uint16_t u16_hight,
                  uint8_t u8_frameRate);


#endif
