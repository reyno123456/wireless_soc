#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debuglog.h"
#include "interrupt.h"
#include "hal_bb.h"
#include "hal_gpio.h"
#include "memory_config.h"
#include "debuglog.h"


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
    uint8_t data_buf[512] ;
    uint32_t i = 0;

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
        for(i = 0; i < 22; i++)
        {
            data_buf[i] = i;
        }

        HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_2, data_buf, 22);
    }
    else if (opt == 2)
    {
        for(i = 0; i < 128; i++)
        {
            data_buf[i] = i;
        }

        HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_2, data_buf, 128);
    }
    else if (opt == 3)
    {
        for(i = 0; i < 512; i++)
        {
            data_buf[i] = i;
        }

        HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_2, data_buf, 512);
    }
}

void BB_grdEventHandler(void *p)
{
    STRU_SysEvent_DEV_BB_STATUS *pstru_status = (STRU_SysEvent_DEV_BB_STATUS *)p;

    if(pstru_status->pid == BB_GET_RCID)
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

    if(pstru_status->pid == BB_GET_RCID)
    {
        dlog_info("Get rcid: 0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x 0x%0.2x", pstru_status->rcid[0], pstru_status->rcid[1], 
                                                                          pstru_status->rcid[2], pstru_status->rcid[3], pstru_status->rcid[4]);
    }
}
