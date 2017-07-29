/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: testhal_pwm.c
Description: 
Author: Artosy Software Team
Version: 0.0.1
Date: 2016/12/19
History:
         0.0.1    2016/12/19    test pwm
*****************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "debuglog.h"
#include "hal_ret_type.h"
#include "hal_pwm.h"
#include "testhal_timer.h"


void commandhal_TestPwm(uint8_t *pu8_pwmNum, uint8_t *pu8_lowus, uint8_t *pu8_highus)
{
    uint32_t u32_PwmNum = strtoul(pu8_pwmNum, NULL, 0);
    uint32_t u32_PwmHigh = strtoul(pu8_highus, NULL, 0);
    uint32_t u32_PwmLow = strtoul(pu8_lowus, NULL, 0);
    
    HAL_PWM_RegisterPwm(u32_PwmNum, u32_PwmLow, u32_PwmHigh);
    HAL_PWM_Start(u32_PwmNum);
    dlog_info("start pwm u32_PwmNum %d  \n",u32_PwmNum);
}
                                                              
void commandhal_TestPwmAll(void)
{
    uint32_t i = 0;
    for(i = 0;i <10; i++)
    {
        HAL_PWM_RegisterPwm(i, 1000, 1000);
        HAL_PWM_Start(i);
    	dlog_info("start pwm u32_PwmNum %d  \n",i);
    }

}
