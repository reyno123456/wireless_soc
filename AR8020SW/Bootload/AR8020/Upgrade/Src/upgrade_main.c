#include <stdint.h>
#include <string.h>
#include "debuglog.h"

#include "upgrade_core.h"
#include "pll_ctrl.h"

#include "hal_ret_type.h"
#include "hal.h"
#include "hal_sys_ctl.h"
#include "hal_nvic.h"
#include "hal_uart.h"

#include "upgrade_command.h"

static void CONSOLE_Init(void)
{
    HAL_UART_Init(DEBUG_LOG_UART_PORT, HAL_UART_BAUDR_115200, NULL);
    DLOG_Init(UPGRADE_CommandRun, NULL, DLOG_SERVER_PROCESSOR);
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{

    SFR_TRX_MODE_SEL = 0x01;
    HAL_SYS_CTL_SetCpuClk(CPU0_CPU1_CORE_PLL_CLK, CPU2_CORE_PLL_CLK);

    HAL_NVIC_SetPriorityGrouping(HAL_NVIC_PRIORITYGROUP_5);
    /* initialize the uart */
    CONSOLE_Init(); //115200 in 200M CPU clock
    dlog_critical("upgrade start!!!\n");
    
    // Default system tick: 1ms.
    HAL_SYS_CTL_SysTickInit(CPU0_CPU1_CORE_PLL_CLK * 1000);

    for( ;; )
    {
        DLOG_Process(NULL);
        HAL_Delay(20);
    }

} 

