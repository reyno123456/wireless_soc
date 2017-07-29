/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: test_os_message.c
Description: 
Author: Wumin @ Artosy Software Team
Version: 0.0.1
Date: 2017/17/19
History:
         0.0.1    2017/07/19    test_os_mutex
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "debuglog.h"
#include "hal.h"
 
static osMutexId stdio_mutex;
static osMutexDef(stdio_mutex);
 
static void notify(const char* name, int state) {
    osMutexWait(stdio_mutex, osWaitForever);
    dlog_info("%s: %d\n\r", name, state);
    osMutexRelease(stdio_mutex);
}
 
static void test_thread(void const *args) 
{
    while (1) 
    {
        notify((const char*)args, 0); 
        osDelay(1000);
/*
        notify((const char*)args, 1); 
        osDelay(1000);
*/
    }
}
 
static void t1(void const *argument) 
{
    while(1)
    {
        osMutexWait(stdio_mutex, osWaitForever);
        dlog_info("got mutex, into critical area");
        HAL_Delay(1000);
        osMutexRelease(stdio_mutex);
        HAL_Delay(1000);
    }
}

 
static void t2(void const *argument) 
{
    while(1)
    {
        osMutexWait(stdio_mutex, osWaitForever);
        dlog_info("got mutex, into critical area");
        HAL_Delay(1000);
        osMutexRelease(stdio_mutex);
        HAL_Delay(1000);
    }
}

static void t3(void const *argument) 
{
    while(1)
    {
        osMutexWait(stdio_mutex, osWaitForever);
        dlog_info("got mutex, into critical area");
        HAL_Delay(1000);
        osMutexRelease(stdio_mutex);
        HAL_Delay(1000);
    }
}

int test_os_mutex() {
    stdio_mutex = osMutexCreate(osMutex(stdio_mutex));

    osThreadDef(OS_mutex_task1, t1, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
    osThreadDef(OS_mutex_task2, t2, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
    osThreadDef(OS_mutex_task3, t3, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(OS_mutex_task1), NULL);
    osThreadCreate(osThread(OS_mutex_task2), NULL);    
    osThreadCreate(osThread(OS_mutex_task3), NULL);    
}
