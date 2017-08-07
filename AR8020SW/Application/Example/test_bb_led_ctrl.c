#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_event.h"
#include "debuglog.h"
#include "interrupt.h"
#include "test_bb_led_ctrl.h"
#include "hal_gpio.h"
#include "memory_config.h"
#include "debuglog.h"
//#include "hal_bb.h"


#define  BLUE_LED_GPIO      (67)
#define  RED_LED_GPIO       (71)


void BB_ledGpioInit(void)
{
    HAL_GPIO_SetMode(RED_LED_GPIO, HAL_GPIO_PIN_MODE2);
    HAL_GPIO_OutPut(RED_LED_GPIO);

    HAL_GPIO_SetMode(BLUE_LED_GPIO, HAL_GPIO_PIN_MODE2);
    HAL_GPIO_OutPut(BLUE_LED_GPIO);

    HAL_GPIO_SetPin(RED_LED_GPIO,  HAL_GPIO_PIN_RESET); //RED LED ON  
    HAL_GPIO_SetPin(BLUE_LED_GPIO, HAL_GPIO_PIN_SET);   //BLUE LED OFF
}

void BB_ledLock(void)
{
    HAL_GPIO_SetPin(BLUE_LED_GPIO, HAL_GPIO_PIN_RESET);     //BLUE LED ON
    HAL_GPIO_SetPin(RED_LED_GPIO, HAL_GPIO_PIN_SET);        //RED LED OFF
}

void BB_ledUnlock(void)
{
    HAL_GPIO_SetPin(BLUE_LED_GPIO, HAL_GPIO_PIN_SET );      //BLUE LED ON
    HAL_GPIO_SetPin(RED_LED_GPIO,  HAL_GPIO_PIN_RESET);     //RED LED OFF
}


void BB_EventHandler(void *p)
{
    STRU_SysEvent_DEV_BB_STATUS *pstru_status = (STRU_SysEvent_DEV_BB_STATUS *)p;

    if (pstru_status->pid == BB_LOCK_STATUS)
    {
        if (pstru_status->lockstatus == 1)
        {
            BB_ledLock();
        }
        else
        {
            BB_ledUnlock();
        }
    }
}
