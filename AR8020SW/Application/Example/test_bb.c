#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debuglog.h"
#include "interrupt.h"
#include "hal_bb.h"
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

void command_test_SkyAutoSearhRcId(void)
{
    extern int BB_add_cmds(uint8_t type, uint32_t param0, uint32_t param1, uint32_t param2);

    BB_add_cmds(16, 0, 0, 0);
}
