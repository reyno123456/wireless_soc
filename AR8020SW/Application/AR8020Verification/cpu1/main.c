#include "debuglog.h"
#include "command.h"
#include "sys_event.h"
#include "hal.h"
#include "hal_sys_ctl.h"
#include "hal_uart.h"
#include "hal_sd.h"
#include "common_func.h"
#include "hal_dma.h"

void CONSOLE_Init(void)
{    
    HAL_UART_Init(DEBUG_LOG_UART_PORT, HAL_UART_BAUDR_115200, NULL);
    DLOG_Init(command_run, (FUNC_LogSave)dlog_output_SD, DLOG_SERVER_PROCESSOR);
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    HAL_SYS_CTL_Init(NULL);

    /* initialize the uart */
    CONSOLE_Init();
    HAL_SD_Init();
    HAL_SD_Fatfs_Init();
/*     HAL_Delay(1000); */
    DLOG_Critical("cpu1 start!!!, time = %s", __TIME__);

	HAL_DMA_init();

    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
        SYS_EVENT_Process();
        HAL_Delay(20);
    }
} 

