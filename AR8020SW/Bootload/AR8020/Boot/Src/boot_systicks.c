#include "boot_systicks.h"
#include "boot_regmap.h"

static volatile uint32_t g_u32SysTickCount = 0;

/**
  * @brief This function is init system tick module.
  * @note ticks should be the CPU frequency, then the count interval should be 1ms.
  * @retval None
  */
void SysTicks_Init(uint32_t ticks)
{
    if ((ticks - 1UL) > SysTick_LOAD_RELOAD_Msk) { return; }    /* Reload value impossible */

    SysTick->LOAD  = (uint32_t)(ticks - 1UL);                         /* set reload register */
    SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                     SysTick_CTRL_TICKINT_Msk   |
                     SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */

    return ;                                                     /* Function successful */
}

/**
  * @brief This function is uninit system tick module.
  * @note None
  * @retval None
  */
void SysTicks_UnInit(void)
{
    SysTick->CTRL  &= ~(SysTick_CTRL_CLKSOURCE_Msk |
                        SysTick_CTRL_TICKINT_Msk   |
                        SysTick_CTRL_ENABLE_Msk);                     /* Disable SysTick IRQ and SysTick Timer */

    return ;                                                     /* Function successful */
}

/**
  * @brief This function is called to increment a global variable "g_u32SysTickCount"
  *        used as application time base.
  * @note In the default implementation, this variable is incremented each in Systick ISR.
  * @retval None
  */
void SysTicks_IncTickCount(void)
{
    g_u32SysTickCount++;
}

/**
  * @brief Provides a tick value
  * @note: Call xTaskGetTickCount instead if the FreeRTOS is running.  
  * @retval Tick value
  */
uint32_t SysTicks_GetTickCount(void)
{
    return g_u32SysTickCount;
}

/**
  * @brief This function provides delay based on variable incremented.
  * @note In the default implementation , SysTick timer is the source of time base.
  *       It is used to generate interrupts at regular time intervals where u32_Tick
  *       is incremented.
  *       call vTaskDelay instead if the FreeRTOS is running.  
  * @param Delay: specifies the delay time length, in milliseconds.
  * @retval None
  */
void SysTicks_DelayMS(uint32_t msDelay)
{
    uint32_t tickstart = 0;
    uint32_t tickcurrent = 0;
    tickstart = g_u32SysTickCount;
    while(1)
    {
        tickcurrent = g_u32SysTickCount;
        if (tickcurrent >= tickstart)
        {
            if ((tickcurrent - tickstart) >= msDelay)
            {
                break;
            }
        }
        else
        {
            if (((MAX_SYS_TICK_COUNT - tickstart) + tickcurrent) >= msDelay)
            {
                break;
            }
        }
    }
}

/**
  * @brief Standard millisecond sleep API.
  * @note
  * @retval None
  */
void msleep(uint32_t millisecs)
{
    SysTicks_DelayMS(millisecs);
}

/**
  * @brief Standard second sleep API.
  * @note
  * @retval None
  */
void ssleep(uint32_t seconds)
{
    SysTicks_DelayMS(seconds * 1000);
}

