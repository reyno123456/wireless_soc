#include "serial.h"
#include "debuglog.h"
#include "test_bb.h"
#include "command.h"
#include "sys_event.h"
#include "hal_sys_ctl.h"
#include "hal_bb.h"
#include "hal.h"
#include "hal_gpio.h"
#include "bb_customerctx.h"
#include "test_bb.h"


void console_init(uint32_t uart_num, uint32_t baut_rate)
{
    dlog_init(command_run, NULL ,DLOG_CLIENT_PROCESSOR);
}


/*
 * should be global data
*/
static STRU_CUSTOMER_CFG stru_user_cfg = 
{
    .flag_useCfgId    =  0,             //use rcid from config
    .rcid             =  {0x20, 0x30, 0x40, 0x50, 0x60},

    .enum_chBandWidth =  BW_10M,
};

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

    HAL_BB_InitGround(&stru_user_cfg);

    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
        SYS_EVENT_Process();
        HAL_Delay(20);
    }
} 

