#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "test_freertos.h"
#include "task.h"
#include "cmsis_os.h"
#include "debuglog.h"
#include "hal.h"
#include "test_os_mail.h"
#include "test_os_mutex.h"
#include "test_os_semaphore.h"
#include "test_os_signal.h"
#include "test_os_message.h"
#include "test_os_timer.h"
#include "test_hal_uart.h"


#define m7_malloc pvPortMalloc
#define m7_free vPortFree

struct AMessage
{
    uint32_t ucMessageID;
    char ucData[100];
};

static QueueHandle_t xQueue1 =NULL;
static QueueHandle_t xQueue2 =NULL;
static SemaphoreHandle_t xMutex;

static void TestTask(void);
static void Task1( void const * argument);
static void Task2( void const * argument);
static void TestQueue(void);
static void queueSendTask(void   const *argument);
static void queueReceiveTask(void const *argument);
static void TestMutex(void);
static void mutexService(void const *argument);
static void TestMem(void);

static void queueSendTask(void   const *argument)
{
/*     struct AMessage* pxMessage = argument; */

	for (;; )
	{
        if( xQueue2 != 0 )
        {
            ((struct AMessage*)argument)->ucMessageID++;        
            xQueueSend( xQueue2, argument, ( TickType_t ) 0 );
			dlog_info("send");
        }

		HAL_Delay(1000);
	}

}

static void queueReceiveTask(void const *argument)
{
	struct AMessage *p_received = malloc(sizeof(struct AMessage));
	if ( 0 == p_received)
	{
		dlog_error("malloc error");
	}

	portBASE_TYPE xStatus;
	uint32_t rev_item;

    for (;;)
	{
		rev_item = uxQueueMessagesWaiting(xQueue2);

		dlog_info("rev_item = %d", rev_item);

		xStatus = xQueueReceive(xQueue2, p_received, 0xFFFFFFFF);
		if (xStatus == pdPASS)
		{
 			dlog_info("receive value is: %s, id = %d", 
 						p_received->ucData, p_received->ucMessageID); 
			memset(p_received,0, sizeof(struct AMessage));
		}
		else
		{
			dlog_warning("Could not receive from the Queue\n");
		}
	}
}

static void TestQueue(void)
{    
    struct AMessage *xMessage1 = malloc(sizeof(struct AMessage));
    memset(xMessage1, 0, sizeof(struct AMessage));
	memcpy(xMessage1->ucData, "test string 11111", strlen("test string 11111"));
/*     xQueue1 = xQueueCreate( 10, sizeof( struct AMessage) ); */

    struct AMessage *xMessage2 = malloc(sizeof(struct AMessage));
    memset(xMessage2, 0, sizeof(struct AMessage));
	memcpy(xMessage2->ucData, "test string 22222", strlen("test string 22222"));

    struct AMessage *xMessage3 = malloc(sizeof(struct AMessage));
    memset(xMessage3, 0, sizeof(struct AMessage));
	memcpy(xMessage3->ucData, "test string 33333", strlen("test string 33333"));
    struct AMessage *xMessage4 = malloc(sizeof(struct AMessage));
    memset(xMessage4, 0, sizeof(struct AMessage));
	memcpy(xMessage4->ucData, "test string 44444", strlen("test string 44444"));
    struct AMessage *xMessage5 = malloc(sizeof(struct AMessage));
    memset(xMessage5, 0, sizeof(struct AMessage));
	memcpy(xMessage5->ucData, "test string 55555", strlen("test string 55555"));
    struct AMessage *xMessage6 = malloc(sizeof(struct AMessage));
    memset(xMessage6, 0, sizeof(struct AMessage));
	memcpy(xMessage6->ucData, "test string 66666", strlen("test string 66666"));
    xQueue2 = xQueueCreate( 10, sizeof( struct AMessage) );

    osThreadDef(Send1_Thread, queueSendTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(Send1_Thread), xMessage1);
    osThreadDef(Send2_Thread, queueSendTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(Send2_Thread), xMessage2);
    osThreadDef(Send3_Thread, queueSendTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(Send3_Thread), xMessage3);
    osThreadDef(Send4_Thread, queueSendTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(Send4_Thread), xMessage4);
    osThreadDef(Send5_Thread, queueSendTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(Send5_Thread), xMessage5);
    osThreadDef(Send6_Thread, queueSendTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(Send6_Thread), xMessage6);

    osThreadDef(Receive_Thread, queueReceiveTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(Receive_Thread), NULL);
}

static void Task1( void const * argument)
{
    static portTickType xLastWakeTime;  
    const portTickType xFrequency = 1000;  

    xLastWakeTime = xTaskGetTickCount();  

    for( ;; )  
    {  
        vTaskDelayUntil( &xLastWakeTime,xFrequency );
        dlog_info("line = %d running", __LINE__);
        dlog_warning("line = %d running", __LINE__);
        dlog_error("line = %d running", __LINE__);
        dlog_critical("line = %d running", __LINE__);
    }  
}

static void Task2( void const * argument)
{
	volatile unsigned long ul;

	for (;;)
	{
        dlog_info("Task2 is running");
		HAL_Delay(3000);
	}
}

static void TestTask(void)
{
	osThreadDef(Task1_Thread, Task1, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(Task1_Thread), NULL);
	osThreadDef(Task2_Thread, Task2, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(Task2_Thread), NULL);
	
	dlog_info("osKernelStart done \n");
}

static void mutexService(void const *argument)
{
	for (;;)
	{
		if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdPASS)
		{
            if (100 == *((const int*)argument))
            {
    			dlog_info("mutexService1 Got Mutex argument %d", *((const int*)argument));
		    }
            else if (200 == *((const int*)argument))
            {
    			dlog_info("mutexService2 Got Mutex argument %d", *((const int*)argument));
            }
        }

		if (xSemaphoreGive(xMutex) == pdTRUE)
		{
			dlog_info("Give the Mutex!\n");
		}

        HAL_Delay(1000);
	}
}

static void TestMutex(void)
{
    uint32_t *args = malloc(sizeof(uint32_t));
    if (args != NULL)
    {
        dlog_info("args = %p", args);
    }

    uint32_t *args2 = malloc(sizeof(uint32_t));
    if (args2 != NULL)
    {
        dlog_info("args = %p", args2);
    }

	xMutex = xSemaphoreCreateMutex();
	if (xMutex == NULL)
	{
		dlog_warning("Create the xMutex failed!\n");
	}
	else
	{
		//xTaskCreate(PrintfMutex, "Print1", 1000, "Task1 gets the Mutex!\n", 1, NULL);
/*
        *args = 100;
		xTaskCreate(mutexService, "mutexService1", 1000, "Task1 gets the Mutex!\n", 1, (void *)args);
        //*args = 200;
		xTaskCreate(mutexService, "mutexService2", 1000, "Task2 gets the Mutex!\n", 2, (void *)args);
*/
        *args = 100;
        osThreadDef(mutexService1, mutexService, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
        osThreadCreate(osThread(mutexService1), args);
        *args2 = 200;
        osThreadDef(mutexService2, mutexService, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
        osThreadCreate(osThread(mutexService2), args2);

	}
}

static void TestMem(void)
{
	unsigned int i;

	void* dynaAddr[5] = { NULL };
	for (i = 0; i < 5; i++)
	{
		dynaAddr[i] = m7_malloc(100);
		dlog_info("malloc addr is 0x%x\n", (unsigned int)dynaAddr[i]);
	}

	for (i = 0; i < 5; i++)
	{
		m7_free(dynaAddr[i]);
		dynaAddr[i] = NULL;
	}
}

void command_TestTask(char* arg1)
{
    uint8_t choise = strtoul(arg1, NULL, 0);
    
    dlog_info("input choise = %d", choise);

    switch(choise)
    {
        case 0:
            TestMem();
        break;

        case 1:
            TestTask();
        break;

        case 2:
            TestQueue();
        break;

        case 3:
            TestMutex();
        break;

        case 4:
            test_os_mail();
        break;

        case 5:
            test_os_mutex();
        break;

        case 6:
            test_os_semaphore();
        break;

        case 7:
            test_os_signal();
        break;

        case 8:
            test_os_message();
        break;

        case 9:
            test_os_timer();
        break;

        case 10:
            test_uart_with_os();
        break;

        default:
        break;
    }
}

void command_TestTaskQuit(void)
{
}
