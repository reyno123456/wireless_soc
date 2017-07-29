/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: testhal_gpio.c
Description: test gpio
Author: SW
Version: 1.0
Date: 2016/12/19
History: test gpio
*****************************************************************************/


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "debuglog.h"
#include "hal_ret_type.h"
#include "hal_gpio.h"

static uint32_t volatile g_u32GpioCount = 0;
static uint32_t volatile g_u32GpioCount2 = 0;

void commandhal_TestGetGpio(uint8_t *gpionum)
{
    uint8_t u8_GpioNum = strtoul(gpionum, NULL, 0);
    uint32_t u32_Gpioval=0;
    HAL_GPIO_InPut(u8_GpioNum);
    HAL_GPIO_GetPin(u8_GpioNum,&u32_Gpioval);
    dlog_info("gpio %d state=%d\n",u8_GpioNum,u32_Gpioval);

}
static void GPIOhal_IRQHandler(uint32_t u32_vectorNum)
{ 
    if(((g_u32GpioCount)%1000 == 0) && (0 !=g_u32GpioCount))
    {

        dlog_info("gpio interrupt0 %d\n",u32_vectorNum);
        dlog_output(1000);
    }
    g_u32GpioCount++;    
}

static void GPIOhal_IRQHandler1(uint32_t u32_vectorNum)
{ 
    
    if(((g_u32GpioCount2)%1000 == 0) && (0 !=g_u32GpioCount2))
    {

        dlog_info("gpio interrupt1 %d\n",u32_vectorNum);
        dlog_output(1000);
    }
    g_u32GpioCount2++;    
}

void commandhal_TestGpioNormal(uint8_t *gpionum, uint8_t *highorlow)
{
	uint8_t u8_GpioNum = strtoul(gpionum, NULL, 0);
    uint8_t u8_GpioValue = strtoul(highorlow, NULL, 0);
    HAL_GPIO_InPut(65);
    HAL_GPIO_OutPut(u8_GpioNum);
    HAL_GPIO_SetPin(u8_GpioNum, u8_GpioValue);	
}

void commandhal_TestGpioInterrupt(uint8_t *gpionum, uint8_t *inttype, uint8_t *polarity)
{
	uint8_t u8_GpioNum = strtoul(gpionum, NULL, 0);
	uint8_t u8_GpioIntType = strtoul(inttype, NULL, 0);
    uint8_t u8_GpioPolarity = strtoul(polarity, NULL, 0);
	
    if(0==HAL_GPIO_RegisterInterrupt(u8_GpioNum, u8_GpioIntType, u8_GpioPolarity, GPIOhal_IRQHandler))
    {
        dlog_info("ok %d",u8_GpioNum);
    }
    else
    {
        dlog_info("fail %d",u8_GpioNum);
    }
    dlog_output(100);
}
