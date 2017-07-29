/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal.h
Description: The external HAL APIs for common functions.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/20
History: 
        0.0.1    2016/12/20    The initial version of hal.h
*****************************************************************************/

#ifndef __HAL_H__
#define __HAL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "hal_ret_type.h"

/**
* @brief  The hal delay function.
* @param  u32_ms    the delay time value in millisecond unit.               
* @retval HAL_OK    means the delay function is well done.
* @note   This function must be called when the system starts.
*/
HAL_RET_T HAL_Delay(uint32_t u32_ms);

/**
* @brief  The hal function to get system tick in millisecond level.
* @param  NONE.
* @retval The current system tick value.
* @note   This function must be called when the system starts.
*/
uint32_t HAL_GetSysMsTick(void);

/**
* @brief  The hal function to get system tick in microsecond level.
* @param  NONE.
* @retval The current system tick value.
* @note   This function must be called when the system starts.
*/
uint64_t HAL_GetSysUsTick(void);

/**
* ar_osDelay will be called only linked with os.lib
*/
__attribute__((weak)) void ar_osDelay(uint32_t u32_ms);

#ifdef __cplusplus
}
#endif

#endif

