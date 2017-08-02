/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_pwm.c
Description: this module contains the helper fucntions necessary to control the general
             purpose pwm block
Author: Artosy Software Team
Version: 0.0.1
Date: 2016/12/19
History:
         0.0.1    2016/12/19    The initial version of hal_pwm.c
*****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "interrupt.h"
#include "hal_ret_type.h"
#include "timer.h"
#include "hal_pwm.h"
#include "debuglog.h"
#include "driver_mutex.h"
/**
* @brief    register pwm
* @param    e_pwmNum: pwm number, the right number should be 0-127.
            u32_lowus: timer load count of low
            u32_highus: timer load count of high
* @retval   HAL_OK                means the registeration pwm success.
*           HAL_GPIO_ERR_UNKNOWN  means the pwm number error. 
* @note     none
*/
HAL_RET_T HAL_PWM_RegisterPwm(ENUM_HAL_PWM_NUM e_pwmNum, uint32_t u32_lowus, uint32_t u32_highus)
{
    if (e_pwmNum > HAL_PWM_NUM9)
    {
        return HAL_PWM_ERR_UNKNOWN;
    }

    if ( -1 == driver_mutex_get(mutex_timer, e_pwmNum) )
    {
        dlog_error("fail, timer for pwm channel = %d", e_pwmNum);
        return HAL_OCCUPIED;
    }
    driver_mutex_set(mutex_timer, (uint32_t)e_pwmNum);

    init_timer_st st_pwm;
    memset(&st_pwm,0,sizeof(init_timer_st));

    st_pwm.base_time_group = e_pwmNum/8;
    st_pwm.time_num = e_pwmNum%8;
    st_pwm.ctrl |= TIME_ENABLE | USER_DEFINED |TIME_PWM_ENABLE;

    TIM_RegisterPwm(st_pwm, u32_lowus, u32_highus);

    return HAL_OK;

}

/**
* @brief    stop pwm
* @param    e_pwmNum: pwm number, the right number should be 0-127.
* @retval   HAL_OK                means the stop pwm success.
*           HAL_GPIO_ERR_UNKNOWN  means the pwm number error. 
* @note     none
*/
HAL_RET_T HAL_PWM_Stop(ENUM_HAL_PWM_NUM e_pwmNum)
{
    if (e_pwmNum > HAL_PWM_NUM9)
    {
        return HAL_PWM_ERR_UNKNOWN;
    }

    init_timer_st st_pwm;
    memset(&st_pwm,0,sizeof(init_timer_st));

    st_pwm.base_time_group = e_pwmNum/8;
    st_pwm.time_num = e_pwmNum%8;
    st_pwm.ctrl |= TIME_ENABLE | USER_DEFINED |TIME_PWM_ENABLE;

    TIM_StopPwm(st_pwm);

    return HAL_OK;
}

/**
* @brief    start pwm
* @param    e_pwmNum: pwm number, the right number should be 0-127.
* @retval   HAL_OK                means the start pwm success.
*           HAL_GPIO_ERR_UNKNOWN  means the pwm number error. 
* @note     none
*/
HAL_RET_T HAL_PWM_Start(ENUM_HAL_PWM_NUM e_pwmNum)
{
    if (e_pwmNum > HAL_PWM_NUM9)
    {
        return HAL_PWM_ERR_UNKNOWN;
    }

    init_timer_st st_pwm;
    memset(&st_pwm,0,sizeof(init_timer_st));

    st_pwm.base_time_group = e_pwmNum/8;
    st_pwm.time_num = e_pwmNum%8;
    st_pwm.ctrl |= TIME_ENABLE | USER_DEFINED |TIME_PWM_ENABLE;

    TIM_StartPwm(st_pwm);

    return HAL_OK;
}
/**
* @brief    dynamic modify pwm duty cycle
* @param    e_pwmNum: pwm number, the right number should be 0-127.
            u32_lowus: timer load count of low
            u32_highus: timer load count of high
* @retval   HAL_OK                means the registeration pwm success.
*           HAL_GPIO_ERR_UNKNOWN  means the pwm number error. 
* @note     none
*/
HAL_RET_T HAL_PWM_DynamicModifyPwmDutyCycle(ENUM_HAL_PWM_NUM e_pwmNum, uint32_t u32_lowus, uint32_t u32_highus)
{
   if (e_pwmNum > HAL_PWM_NUM9)
    {
        return HAL_PWM_ERR_UNKNOWN;
    }

    init_timer_st st_pwm;
    memset(&st_pwm,0,sizeof(init_timer_st));

    st_pwm.base_time_group = e_pwmNum/8;
    st_pwm.time_num = e_pwmNum%8;
    st_pwm.ctrl |= TIME_ENABLE | USER_DEFINED |TIME_PWM_ENABLE;

    TIM_ModifyPwmCount(st_pwm,u32_lowus,u32_highus);
}
