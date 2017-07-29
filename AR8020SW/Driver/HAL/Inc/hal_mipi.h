/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_mipi.h
Description: The external HAL APIs to use the mipi controller.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2017/01/23
History: 
        0.0.1    2017/01/23    The initial version of hal_mipi.h
*****************************************************************************/
#ifndef __HAL_MIPI_H__
#define __HAL_MIPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "hal_ret_type.h"


/**
* @brief    
* @param     
* @retval  
* @note    
*/
HAL_RET_T HAL_MIPI_Init(uint8_t u8_toEncoderCh, 
                        uint16_t u16_width,
                        uint16_t u16_hight,
                        uint8_t u8_frameRate);

#ifdef __cplusplus
}
#endif

#endif
