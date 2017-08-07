#include "serial.h"
#include "debuglog.h"
#include "command.h"
#include "sys_event.h"
#include "hal_sys_ctl.h"
#include "hal_bb.h"
#include "hal.h"
#include "test_bb.h"


extern int BB_add_cmds(uint8_t type, uint32_t param0, uint32_t param1, uint32_t param2);

void console_init(uint32_t uart_num, uint32_t baut_rate)
{
    dlog_init(command_run, NULL, DLOG_CLIENT_PROCESSOR);
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{ 
    STRU_HAL_SYS_CTL_CONFIG *pst_cfg;
    HAL_SYS_CTL_GetConfig(&pst_cfg);
    HAL_SYS_CTL_Init(pst_cfg);
   
    console_init(2, 115200);
    dlog_set_output_level(LOG_LEVEL_WARNING);
    dlog_critical("main ground function start \n");

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_BB_EVENT, BB_grdEventHandler);

    HAL_BB_InitGround(NULL);
    
    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
        SYS_EVENT_Process();
        HAL_Delay(20);
    }
} 


