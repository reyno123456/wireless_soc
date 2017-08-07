#include "debuglog.h"
#include "serial.h"
#include "command.h"
#include "test_usbh.h"
#include "cmsis_os.h"
#include "sys_event.h"
#include "bb_spi.h"
#include "hal.h"
#include "hal_gpio.h"
#include "hal_bb.h"
#include "hal_hdmi_rx.h"
#include "hal_usb_otg.h"
#include "hal_sys_ctl.h"
#include "wireless_interface.h"
#include "hal_nv.h"
#include "hal_usb_host.h"
#include "hal_softi2s.h"
#include "hal_encodemp3.h"
#include "systicks.h"
#include "memory_config.h"
#include "hal_uart.h"
#include "it_typedef.h"
#include "it6602.h"
#include "hal_sram.h"
#include "arcast_appcommon.h"
#include "hal_pmu.h"
#include "hal_timer.h"
#include "test_bb_led_ctrl.h"


void TIMER_avsyncInterruptHandle(uint32_t u32_vectorNum)
{
    volatile uint32_t u32_tick = *((volatile uint32_t *)(SRAM_MODULE_SHARE_AVSYNC_TICK));
    u32_tick++;
    *((volatile uint32_t *)(SRAM_MODULE_SHARE_AVSYNC_TICK)) = u32_tick;

}

uint8_t Command_FormatStatus(void);
void CONSOLE_Init(void)
{
    HAL_UART_Init(DEBUG_LOG_UART_PORT, HAL_UART_BAUDR_115200, NULL);
    DLOG_Init(command_run, NULL, DLOG_SERVER_PROCESSOR);
}

void HDMI_powerOn(void)
{
    HAL_GPIO_OutPut(HAL_GPIO_NUM59);
    HAL_GPIO_SetPin(HAL_GPIO_NUM59, HAL_GPIO_PIN_SET);
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */

int main(void)
{
    STRU_HAL_SYS_CTL_CONFIG *pst_cfg;
    HAL_SYS_CTL_GetConfig( &pst_cfg);
    pst_cfg->u8_workMode = 0;
    HAL_SYS_CTL_Init(pst_cfg);
    
    /* initialize the uart */
    dlog_set_output_level(LOG_LEVEL_INFO);
    CONSOLE_Init();
    dlog_critical("cpu0 start!!! \n");

    BB_ledGpioInit();
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_BB_EVENT, BB_EventHandler);
    
    //HAL_PMU_Init();
    
    HAL_GPIO_InPut(HAL_GPIO_NUM99);

    HAL_USB_ConfigPHY();

    HDMI_powerOn();

    *((uint32_t *)(SRAM_MODULE_SHARE_AVSYNC_TICK)) = 0;
    HAL_TIMER_RegisterTimer(HAL_TIMER_NUM21, 1000, TIMER_avsyncInterruptHandle);

    *((uint8_t *)(SRAM_MODULE_SHARE_AUDIO_PCM)) = HAL_SOFTI2S_ENCODE_IEC_48000;
    STRU_MP3_ENCODE_CONFIGURE_WAVE st_audioConfig;
    st_audioConfig.e_samplerate = HAL_MP3_ENCODE_48000;
    st_audioConfig.e_modes = HAL_MP3_ENCODE_STEREO;
    st_audioConfig.u32_rawDataAddr = AUDIO_DATA_START;
    st_audioConfig.u32_rawDataLenght = AUDIO_DATA_BUFF_SIZE;
    st_audioConfig.u32_encodeDataAddr = MPE3_ENCODER_DATA_ADDR;
    st_audioConfig.u32_newPcmDataFlagAddr = SRAM_MODULE_SHARE_AUDIO_PCM;
    st_audioConfig.u8_channel = 2;
    HAL_MP3EncodePcmInit(&st_audioConfig, 0);
    

    STRU_HDMI_CONFIGURE        st_configure;
    st_configure.e_getFormatMethod = HAL_HDMI_INTERRUPT;
    st_configure.st_interruptGpio.e_interruptGpioNum = HAL_GPIO_NUM98;
    st_configure.st_interruptGpio.e_interruptGpioPolarity = HAL_GPIO_ACTIVE_LOW;
    st_configure.st_interruptGpio.e_interruptGpioType = HAL_GPIO_LEVEL_SENUMSITIVE;
    st_configure.u8_hdmiToEncoderCh = 1;
    HAL_HDMI_RX_Init(HAL_HDMI_RX_1, &st_configure);

    HAL_USB_InitOTG(HAL_USB_PORT_0);

    HAL_USB_InitOTG(HAL_USB_PORT_1);

    HAL_NV_Init();
    Wireless_TaskInit(WIRELESS_NO_RTOS);

    Common_AVFORMATSysEventSKYInit();
    
    for( ;; )
    {
        if (ARCAST_SKY_FORMAT_STABLE == Command_FormatStatus())
        {
            HAL_MP3EncodePcm();
        }
        Wireless_MessageProcess();
        SYS_EVENT_Process();
    }
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

