#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "debuglog.h"
#include "hal_ret_type.h"
#include "hal_timer.h"
#include "testhal_timer.h"

static uint32_t g_u32TimCount1 = 0;
static uint32_t g_u32TimCount2 = 0;

void TIMHAL_IRQHandler0(uint32_t u32_vectorNum)
{ 
    if(((g_u32TimCount1)%1000 == 0) && (0 !=g_u32TimCount1))
    {

        dlog_info("g_u32TimCount1 %d\n",u32_vectorNum);
    }
    g_u32TimCount1++;

}

void TIMHAL_IRQHandler1(uint32_t u32_vectorNum)
{

    if(((g_u32TimCount2)%1000 == 0) && (0 !=g_u32TimCount2))
    {

        dlog_info("g_u32TimCount2 %d\n",u32_vectorNum);
    }
    g_u32TimCount2++;

}

static void Test_TimerInit(uint8_t u8_timerNum,uint8_t u8_timerCount)
{

    HAL_TIMER_RegisterTimer(u8_timerNum, u8_timerCount, TIMHAL_IRQHandler0);
}

void commandhal_TestTim(uint8_t *pu8_timerNum, uint8_t *pu8_timerCount)
{

    uint32_t u32_TimNum = strtoul(pu8_timerNum, NULL, 0);
    uint32_t u32_TimCount = strtoul(pu8_timerCount, NULL, 0);
    
    HAL_TIMER_RegisterTimer(u32_TimNum, u32_TimCount*1000, TIMHAL_IRQHandler0);
    HAL_TIMER_RegisterTimer(u32_TimNum+1, u32_TimCount*500, TIMHAL_IRQHandler1);
}
                                                              
void commandhal_TestTimAll(void)
{
    /*uint32_t i = 0;    
    for(i = 0;i <24; i++)
    {
        Test_TimerInit(i,1000);
    
        while(g_u32TimCount0 < 5000)
        {
            if((g_u32TimCount0)%1000 == 0)
                dlog_info("timer count %d \n", g_u32TimCount0);
        }   
   
        TIM_StopTimer(i); 
        g_u32TimCount0 = 0;
    }*/
    
}
