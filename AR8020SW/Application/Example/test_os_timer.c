/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: test_os_message.c
Description: 
Author: Wumin @ Artosy Software Team
Version: 0.0.1
Date: 2017/17/19
History:
         0.0.1    2017/07/19    test_os_timer
note:
1. argument of osTimerCreate is not achieve in cmsis_os.c
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "debuglog.h"
#include "hal.h"
#include "test_os_timer.h"

typedef struct {
    uint32_t    voltage;
    uint32_t    current;
    uint32_t    counter;
} message_t;

message_t *p_message1;
message_t *p_message2;
message_t *p_message3;
message_t *p_message4;

static void blink(void const *argument) 
{
    dlog_info("voltage = %d", ((message_t*)argument)->voltage);
    dlog_info("current = %d", ((message_t*)argument)->current);    
    dlog_info("counter = %d", ((message_t*)argument)->counter);
}

void test_os_timer(void)
{
    p_message1 = malloc(sizeof(message_t));
    p_message2 = malloc(sizeof(message_t));
    p_message3 = malloc(sizeof(message_t));
    p_message4 = malloc(sizeof(message_t));

    p_message1->counter = 1;
    p_message1->current = 2;
    p_message1->voltage = 3;

    p_message2->counter = 11;
    p_message2->current = 22;
    p_message2->voltage = 33;

    p_message3->counter = 111;
    p_message3->current = 222;
    p_message3->voltage = 333;

    p_message4->counter = 1111;
    p_message4->current = 2222;
    p_message4->voltage = 3333;

    osTimerDef(blink_0, blink);
    osTimerDef(blink_1, blink);
    osTimerDef(blink_2, blink);
    osTimerDef(blink_3, blink);

    osTimerId timer_0 = osTimerCreate(osTimer(blink_0), osTimerPeriodic, (void *)p_message1);
    osTimerId timer_1 = osTimerCreate(osTimer(blink_1), osTimerPeriodic, (void *)p_message2);
    osTimerId timer_2 = osTimerCreate(osTimer(blink_2), osTimerPeriodic, (void *)p_message3);
    osTimerId timer_3 = osTimerCreate(osTimer(blink_3), osTimerPeriodic, (void *)p_message4);

    osTimerStart(timer_0, 10000);    
    osTimerStart(timer_1, 1000);
    osTimerStart(timer_2,  500);
    osTimerStart(timer_3,  250);
}
