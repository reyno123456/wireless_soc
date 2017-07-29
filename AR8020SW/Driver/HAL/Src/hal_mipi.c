/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_mipi.c
Description: The external HAL APIs to use the mipi controller.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2017/01/23
History: 
        0.0.1    2017/01/23    The initial version of hal_mipi.c
*****************************************************************************/
#include "hal_mipi.h"
#include "mipi.h"
#include <stdio.h>


/**
* @brief    
* @param     
* @retval  
* @note    
*/
HAL_RET_T HAL_MIPI_Init(uint8_t u8_toEncoderCh, 
                        uint16_t u16_width,
                        uint16_t u16_hight,
                        uint8_t u8_frameRate)

{
    int32_t s_result;

    s_result = MIPI_Init(u8_toEncoderCh, 
                        u16_width,
                        u16_hight,
                        u8_frameRate);
    
    if (0 == s_result)
    {
        return HAL_OK;
    }
    else
    {
        return HAL_MIPI_ERR;
    }
}
