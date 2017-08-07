/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: test_os_message.c
Description: 
Author: Wumin @ Artosy Software Team
Version: 0.0.1
Date: 2017/17/19
History:
         0.0.1    2017/07/19    test_os_mail
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "debuglog.h"
#include "hal.h"

struct Thread0_Mail
{
    int a;
    int b;
};
struct Thread1_Mail
{
    int x;
    int y;
};

static osMailQId  thread1_mail;
static osMailQId  thread0_mail;


/* Forward reference */
static void os_mail_send_thread (void const *argument);
static void os_mail_recv_thread (void const *argument);

static void os_mail_send_thread (void const *argument) 
{
    osStatus ret;
    osEvent event;
    int x = 100;
    int y = 1000;
    struct Thread1_Mail * Thread1_mail;

    while ( 1 )
    {
        Thread1_mail = osMailAlloc( thread1_mail, osWaitForever );
        if (NULL == Thread1_mail)
        {
            dlog_error("error");
        }

        if ( Thread1_mail )
        {
            Thread1_mail->x = x++;
            Thread1_mail->y = y++;
            ret = osMailPut( thread0_mail, Thread1_mail );
            if (osOK == ret)
            {       
                dlog_info("send ok");
            }
            else
            {
                dlog_error("send error");
            }
            
            event = osMailGet( thread1_mail, osWaitForever );

            if ( event.status == osEventMail )
            {
                osMailFree( thread1_mail, Thread1_mail );
            }
        }

        HAL_Delay( 1000 );
    }
}

static void os_mail_recv_thread (void const *argument) 
{
    osEvent event;
    struct Thread0_Mail * Thread0_mail;

    while ( 1 )
    {
        event = osMailGet( thread0_mail, osWaitForever );

        if ( event.status == osEventMail )
        {
            struct Thread0_Mail *p_tmp = event.value.p;
            dlog_info("received data a = %d", ((struct Thread0_Mail*)(event.value.p))->a);
            dlog_info("received data b = %d", ((struct Thread0_Mail*)(event.value.p))->b);
            Thread0_mail = osMailAlloc( thread0_mail, osWaitForever ); 

            if ( Thread0_mail )
            {
                Thread0_mail->a = 100;
                Thread0_mail->b = 1000;
                osMailPut( thread1_mail, Thread0_mail );
                osMailFree( thread0_mail, Thread0_mail );
            }
        }

        HAL_Delay( 10 );
    }
}

void test_os_mail(void)
{
	osMailQDef( thread0_mail, 10, struct Thread0_Mail );
	osMailQDef( thread1_mail, 10, struct Thread1_Mail );
	thread0_mail = osMailCreate(osMailQ(thread0_mail),0);
	thread1_mail = osMailCreate(osMailQ(thread1_mail),0);
    osThreadDef(OS_MAIL_SEND, os_mail_send_thread, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
    osThreadDef(OS_MAIL_RECV, os_mail_recv_thread, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(OS_MAIL_SEND), NULL);
    osThreadCreate(osThread(OS_MAIL_RECV), NULL);
}
