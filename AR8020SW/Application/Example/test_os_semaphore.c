/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: test_os_message.c
Description: 
Author: Wumin @ Artosy Software Team
Version: 0.0.1
Date: 2017/17/19
History:
         0.0.1    2017/07/19    test_os_semaphore
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "debuglog.h"
#include "hal.h"

static osSemaphoreId two_slots;
 

static void semaphore_task2(void const *argument) 
{
    while(1)
    {
        osSemaphoreWait(two_slots, 5000);
        dlog_info("into critical");
        HAL_Delay(1000);
        osSemaphoreRelease(two_slots);
        osThreadYield();
    }
}
 
static void semaphore_task3(void const *argument) 
{
    while(1)
    {
        osSemaphoreWait(two_slots, osWaitForever);
        dlog_info("into critical");
        HAL_Delay(1000);
        osThreadYield();
    }
}

int test_os_semaphore(void)
{
    osSemaphoreDef(two_slots);
    two_slots = osSemaphoreCreate(osSemaphore(two_slots), 1);
    osThreadDef(OS_semaphore_task2, semaphore_task2, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(OS_semaphore_task2), NULL);
    osThreadDef(OS_semaphore_task3, semaphore_task3, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(OS_semaphore_task3), NULL);
}
