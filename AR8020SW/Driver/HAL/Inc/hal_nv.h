#ifndef __HAL_NV_H__
#define __HAL_NV_H__

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
HAL_RET_T HAL_NV_Init(void);

/**
* @brief      
* @param    
* @retval   
* @note   
*/
HAL_RET_T HAL_NV_ResetBbRcId(void);

/**
* @brief      
* @param    
* @retval   
* @note   
*/
HAL_RET_T HAL_NV_SetBbRcId(uint8_t *u8_bbRcId);


#ifdef __cplusplus
}
#endif

#endif

