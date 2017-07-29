/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: test_os_message.c
Description: 
Author: Wumin @ Artosy Software Team
Version: 0.0.1
Date: 2017/17/19
History:
         0.0.1    2017/07/19    test_os_signal
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "debuglog.h"
#include "hal.h"

#define NOTIFY_VALUE 0x0001

static osThreadId g_tid; 
static void os_signal_task1(void const *argument) 
{
    osEvent eve;
    while(1)
    {
        eve = osSignalWait(0x2, osWaitForever);
		dlog_info("eve.status = 0x%x", eve.status);
        if (eve.value.signals == NOTIFY_VALUE)
        {
            dlog_info("%d, got signal 0x%08x", __LINE__, eve.value.signals);
        }
        else
        {
            dlog_info("%d, got signal 0x%08x", __LINE__, eve.value.signals);
            //osSignalClear(g_tid, eve.value.signals);
        }
    }
}
 
static void os_signal_task2(void const *argument) 
{
    while(1)
    {
        HAL_Delay(1000);
        dlog_info("give signal");
        osSignalSet(g_tid, NOTIFY_VALUE);
    }
}

void test_os_signal(void)
{
    // tid = osThreadCreate(osThread(led_thread), NULL);
    osThreadDef(OS_SIGNAL_task1, os_signal_task1, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
    g_tid = osThreadCreate(osThread(OS_SIGNAL_task1), NULL);
    osThreadDef(OS_SIGNAL_task2, os_signal_task2, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(OS_SIGNAL_task2), NULL);
}
