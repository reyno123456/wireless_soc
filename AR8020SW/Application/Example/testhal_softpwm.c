/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: test_simulatewm.c
Description: test softpwm
Author: SW
Version: 1.0
Date: 2016/12/19
History: test softpwm
*****************************************************************************/


#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "debuglog.h"
#include "hal_ret_type.h"
#include "hal_softpwm.h"

void commandhal_TestSimulatePwm(void)
{
    STRU_SoftPwmHandle tmp;
    uint32_t i = 0;
    uint32_t h = 1;
    uint32_t tmp2[10]={0};
    memset(&tmp,0,sizeof(STRU_SoftPwmHandle)*10);
    memset(tmp2,1,sizeof(uint32_t)*10);
    
    HAL_SOFTPWM_SetTimer(4);

    memset(&tmp,0,sizeof(STRU_SoftPwmHandle));
    
    tmp.u32_countArray[0] =100;
    tmp.u32_countArray[1] =200;
    tmp.u8_pin = 64;
    tmp.function = HAL_GPIO_SetPin;


    HAL_GPIO_OutPut(64);
    HAL_SOFTPWM_AddPwm(&tmp);

    for (i=90;i<95;i++)
    {
        tmp.u32_countArray[0] =100+i*10;
        tmp.u32_countArray[1] =200+i*10;
        tmp.u8_pin = i;
        HAL_GPIO_OutPut(i);
        HAL_SOFTPWM_AddPwm(&tmp);
    }
    for (i=0;i<MAXSOFTPWM;i++)
    {
        g_ACount[i][0] = 0;
        g_ACount[i][1] = 0;
    }
    while (1)
    {
        HAL_SOFTPWM_RunPwm();
        for (i=0;i<MAXSOFTPWM;i++)
        {
            if ((g_stPwmQueue[i].u8_polarity == 0) &&
                (g_ACount[i][0] == 0)) 
            {
                tmp2[i]++;
                if(tmp2[i]>2)
                {
                     tmp2[i]=1;
                }
                g_ACount[i][0] =40* tmp2[i];
                g_ACount[i][1] =50* tmp2[i];        
            }
        } 
        dlog_output(100);

    }
}