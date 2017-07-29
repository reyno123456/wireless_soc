#include "debuglog.h"
#include "command.h"
#include "sys_event.h"
#include "serial.h"
#include "hal.h"
#include "hal_sys_ctl.h"
#include "hal_softi2s.h"
#include "hal_uart.h"



void CONSOLE_Init(void)
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
    HAL_SYS_CTL_Init(NULL);
    dlog_set_output_level(LOG_LEVEL_INFO);
    CONSOLE_Init();        
    dlog_critical("cpu1 start!!!\n"); 
    STRU_HAL_SOFTI2S_INIT st_audioConfig = {AUDIO_DATA_START,HAL_GPIO_NUM35,HAL_GPIO_NUM70,HAL_GPIO_NUM20};
    HAL_SOFTI2S_Init(&st_audioConfig);
    HAL_SOFTI2S_Funct();
} 

