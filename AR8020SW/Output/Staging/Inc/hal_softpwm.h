/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_softpwm.h
Description: this module contains the helper fucntions necessary to control the general
             purpose softpwm block.softpwm use a timer to toggle gpio create pwm.
Author: Artosy Software Team
Version: 0.0.1
Date: 2016/12/19
History:
         0.0.1    2016/12/19    The initial version of hal_softpwm.h
*****************************************************************************/

#ifndef __HAL_SOFTPWM_H__
#define __HAL_SOFTPWM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define MAXSOFTPWM         (10)

#define MIXMODIFYTIMEUM    (20)
#include "hal_timer.h"
#include "hal_gpio.h"

typedef struct
{
   
    uint32_t u32_countArray[2];        //[0] pwm high [1] pwm low
    uint32_t u32_overFlow;              //if over 0xFFFFFFFF
    uint32_t u32_baseTime;              //add pwm timer    
    uint8_t  u8_polarity;               //low or high
    uint8_t  u8_pin;                    // toggle gpio    
    HAL_RET_T (*function)(ENUM_HAL_GPIO_NUM,ENUM_HAL_GPIO_PinState);//callbak  

} STRU_SoftPwmHandle;

//g_ACount[MAXSOFTPWM][0] gpio high
//g_ACount[MAXSOFTPWM][1] gpio low
extern uint32_t g_ACount[MAXSOFTPWM][2];

extern STRU_SoftPwmHandle g_stPwmQueue[MAXSOFTPWM];
/**
* @brief    register tiemr and start timer
* @param    e_timerNum: timer number
* @retval   HAL_OK : function is well done.
* @note     none
*/
HAL_RET_T HAL_SOFTPWM_SetTimer(ENUM_HAL_TIMER_NUM e_timerNum);

/**
* @brief    add a simulatepwm
* @param    e_timerNum: pwm number
* @retval   HAL_SOFTPWM_ERR_GPIOMAX : softpwm over MAXSOFTPWM.
            HAL_OK : function is well done.
* @note     none
*/
HAL_RET_T HAL_SOFTPWM_AddPwm(STRU_SoftPwmHandle *tmp);
/**
* @brief    while toggle pin to creat soft pwm
* @param    none
* @retval   HAL_OK : function is well done. 
* @note     none
*/
HAL_RET_T HAL_SOFTPWM_RunPwm(void);

#ifdef __cplusplus
}
#endif

#endif
