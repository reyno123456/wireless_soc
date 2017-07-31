#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debuglog.h"
#include "interrupt.h"
#include "hal_bb.h"
#include "hal_gpio.h"
#include "memory_config.h"
#include "debuglog.h"
#include "hal_bb.h"


static void rcvDataHandler(void *p)
{
    uint8_t data_buf_proc[1024];
    uint32_t u32_rcvLen = 0;
    
    HAL_BB_UartComReceiveMsg(BB_UART_COM_SESSION_2, data_buf_proc, sizeof(data_buf_proc), &u32_rcvLen);

    dlog_info("rcv: %d", u32_rcvLen);
    dlog_info("%d %d", data_buf_proc[0], data_buf_proc[1]);
}


void command_test_BB_uart(char *index_str)
{
    unsigned char opt = strtoul(index_str, NULL, 0);

    if (opt == 0)
    {
        HAL_BB_UartComRemoteSessionInit();
        HAL_BB_UartComRegisterSession(BB_UART_COM_SESSION_2, 
                                      BB_UART_SESSION_PRIORITY_HIGH,
                                      BB_UART_SESSION_DATA_NORMAL,
                                      rcvDataHandler);
    }
    else if (opt == 1)
    {
        uint8_t data_buf[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22};
        HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_2, data_buf, sizeof(data_buf));
    }
    else if (opt == 2)
    {
        uint32_t i = 0;
        uint8_t data_buf_proc[128] ;
        for(i = 0; i < sizeof(data_buf_proc); i++)
        {
            data_buf_proc[i] = i;
        }

        HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_2, data_buf_proc, sizeof(data_buf_proc));
    }
}


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


void BB_grdEventHandler(void *p)
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
    else if(pstru_status->pid == BB_GET_RCID)
    {
        dlog_info("Get rcid: 0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x", 
                             pstru_status->rcid[0], pstru_status->rcid[1], 
                             pstru_status->rcid[2], pstru_status->rcid[3], pstru_status->rcid[4]);

        HAL_BB_GroundDisConnectSkyByRcId(pstru_status->rcid);
    }
}


void BB_skyEventHandler(void *p)
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
    else if(pstru_status->pid == BB_GET_RCID)
    {
        dlog_warning("Get rcid: 0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x", pstru_status->rcid[0], pstru_status->rcid[1], 
                                                                          pstru_status->rcid[2], pstru_status->rcid[3], pstru_status->rcid[4]);
    }
}
