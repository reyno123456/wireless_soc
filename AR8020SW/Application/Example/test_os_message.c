/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: test_os_message.c
Description: 
Author: Wumin @ Artosy Software Team
Version: 0.0.1
Date: 2017/17/19
History:
         0.0.1    2017/07/19    test_os_message
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "debuglog.h"
#include "hal.h"
#include "test_os_message.h"

typedef struct {
    uint32_t    voltage;
    uint32_t    current;
    uint32_t    counter;
} message_t;
 
static osPoolId  mpool; 
static osMessageQId  queue;
 
static void os_message_task1 (void const *args) 
{
    uint32_t i = 0;
    while (1) 
    {
        i++;
        message_t *message = (message_t*)osPoolAlloc(mpool);
        message->voltage = i * 33; 
        message->current = i * 11;
        message->counter = i;
        osMessagePut(queue, (uint32_t)message, osWaitForever);
        HAL_Delay(1000);
    }
}
  
static void os_message_task2 (void const *args)
{
    while (1) 
    {
        osEvent evt = osMessageGet(queue, osWaitForever);
        if (evt.status == osEventMessage) 
        {
            message_t *message = (message_t*)evt.value.p;
            dlog_info("Voltage: %d V"   , message->voltage);
            dlog_info("Current: %d A"     , message->current);
            dlog_info("Number of cycles: %u", message->counter);
            osPoolFree(mpool, message);
        }
    }
}

void test_os_message(void)
{
    osPoolDef(mpool, 16, uint32_t);
    osMessageQDef(queue, 16, uint32_t);

    mpool = osPoolCreate(osPool(mpool));
    queue = osMessageCreate(osMessageQ(queue), NULL);

    osThreadDef(OS_MESSAGE_task1, os_message_task1, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(OS_MESSAGE_task1), NULL);

    osThreadDef(OS_MESSAGE_task2, os_message_task2, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(OS_MESSAGE_task2), NULL);
}

#if 0
/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: test_os_message.c
Description: 
Author: Wumin @ Artosy Software Team
Version: 0.0.1
Date: 2017/17/19
History:
         0.0.1    2017/07/19    test_os_message
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "debuglog.h"
#include "hal.h"
#include "test_os_message.h"

typedef struct {
    uint32_t    voltage;
    uint32_t    current;
    uint32_t    counter;
} message_t;

static osMessageQId thread0_MsgQ;
static osMessageQId thread1_MsgQ;

static void os_message_task1( const void *arg)
{
  osEvent event;
  message_t *send_msg = (message_t*)malloc(sizeof(message_t));
  send_msg->counter = 1;
  send_msg->current = 2;
  send_msg->voltage =3;

  while(1)
  {
    event = osMessageGet(thread0_MsgQ, osWaitForever );
    if ( event.status == osEventMessage)
    {
        dlog_info("get message, %d", ((message_t *)(event.value.p))->counter);
		send_msg->counter++;
		osMessagePut( thread1_MsgQ, (uint32_t)send_msg, osWaitForever );
    }
    osDelay( 1000 );
  }
}

static void os_message_task2( const void *arg)
{
    osEvent event;
	message_t *send_msg = (message_t*)malloc(sizeof(message_t));
	send_msg->counter = 1;
	send_msg->current = 2;
	send_msg->voltage =3;
    // osMessagePut( thread0_MsgQ, event.value.v+1, osWaitForever );
   	osMessagePut( thread0_MsgQ, (uint32_t)(send_msg), osWaitForever );

    while(1)
    {
        event = osMessageGet(thread1_MsgQ, osWaitForever );
        if ( event.status == osEventMessage)
        {
			dlog_info("get message, %d", ((message_t *)(event.value.p))->counter);
			send_msg->counter++;
            osMessagePut( thread0_MsgQ, (uint32_t)send_msg, osWaitForever );
        }
        osDelay( 1000 );
    }
}

void test_os_message( void )
{
    osMessageQDef(thread0_MsgQ, 16, uint32_t);
    osMessageQDef(thread1_MsgQ, 16, uint32_t);
    thread0_MsgQ = osMessageCreate(osMessageQ(thread0_MsgQ), NULL);
    thread1_MsgQ = osMessageCreate(osMessageQ(thread1_MsgQ), NULL);

    osThreadDef(OS_MESSAGE_task1, os_message_task1, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(OS_MESSAGE_task1), (void *)100);

    osThreadDef(OS_MESSAGE_task2, os_message_task2, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(OS_MESSAGE_task2), (void *)200);
}
#endif

#if 0
/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: test_os_message.c
Description: 
Author: Wumin @ Artosy Software Team
Version: 0.0.1
Date: 2017/17/19
History:
         0.0.1    2017/07/19    test_os_message
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "debuglog.h"
#include "hal.h"
#include "test_os_message.h"

typedef struct {
    uint32_t    voltage;
    uint32_t    current;
    uint32_t    counter;
} message_t;
 
// static osPoolId  mpool; 
static osMessageQId  queue;
 
static void os_message_task1 (void const *args) 
{
    uint32_t i = 0;
    while (1) 
    {
        i++;
        // message_t *message = (message_t*)osPoolAlloc(mpool);
        message_t *message = (message_t*)malloc(sizeof(message_t));
        message->voltage = i * 33; 
        message->current = i * 11;
        message->counter = i;
        osMessagePut(queue, (uint32_t)message, osWaitForever);
        HAL_Delay(1000);
    }
}
  
static void os_message_task2 (void const *args)
{
    while (1) 
    {
        osEvent evt = osMessageGet(queue, osWaitForever);
        if (evt.status == osEventMessage) 
        {
            message_t *message = (message_t*)evt.value.p;
            dlog_info("Voltage: %d V"   , message->voltage);
            dlog_info("Current: %d A"     , message->current);
            dlog_info("Number of cycles: %u", message->counter);
            //osPoolFree(mpool, message);
			free(message);
        }
    }
}

void test_os_message(void)
{
    // osPoolDef(mpool, 16, uint32_t);
    osMessageQDef(queue, 16, uint32_t);

    // mpool = osPoolCreate(osPool(mpool));
    queue = osMessageCreate(osMessageQ(queue), NULL);

    osThreadDef(OS_MESSAGE_task1, os_message_task1, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(OS_MESSAGE_task1), NULL);

    osThreadDef(OS_MESSAGE_task2, os_message_task2, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(OS_MESSAGE_task2), NULL);
}
#endif

