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

#ifndef __HAL_TEST_PWM_H__
#define __HAL_TEST_PWM_H__

void commandhal_TestPwm(uint8_t *pu8_pwmNum, uint8_t *pu8_lowus, uint8_t *pu8_highus);
                                                              
void commandhal_TestPwmAll(void);

#endif