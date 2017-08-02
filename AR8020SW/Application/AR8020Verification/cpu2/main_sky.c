#include "serial.h"
#include "debuglog.h"
#include "hal_h264.h"
#include "hal_bb.h"
#include "command.h"
#include "sys_event.h"
#include "hal_sys_ctl.h"
#include "hal.h"
#include "test_bb.h"

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

    /* initialize the uart */
    console_init(2, 115200);   
    dlog_critical("cpu2 start, time = %s\n", __TIME__);

/*
    STRU_HAL_H264_CONFIG st_h264Cfg;
    st_h264Cfg.u8_view0En = 1;
    st_h264Cfg.u8_view0Gop = 10;
    st_h264Cfg.e_view0Br = HAL_H264_BITRATE_500K;
    st_h264Cfg.u8_view0BrEn = 1;
    st_h264Cfg.u8_view1En = 1;
    st_h264Cfg.u8_view1Gop = 10;
    st_h264Cfg.e_view1Br = HAL_H264_BITRATE_500K;
    st_h264Cfg.u8_view1BrEn = 1;
    HAL_H264_Init(st_h264Cfg);

    BB_ledGpioInit();
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_BB_EVENT, BB_skyEventHandler);
    
    HAL_BB_InitSky(NULL);
*/

	dlog_set_output_level(LOG_LEVEL_WARNING);

    for( ;; )
    {
        SYS_EVENT_Process();
        HAL_Delay(20);
    }
} 

