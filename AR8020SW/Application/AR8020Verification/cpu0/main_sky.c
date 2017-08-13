#include "debuglog.h"
#include "serial.h"
#include "test_freertos.h"
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
#include "hal_dma.h"
#include "hal_sd.h"
#include "test_bb_led_ctrl.h"
#include "lwip/tcpip.h"


void console_init(uint32_t uart_num, uint32_t baut_rate)
{
    dlog_init(command_run, NULL, DLOG_CLIENT_PROCESSOR);
}

void HDMI_powerOn(void)
{
    HAL_GPIO_OutPut(63);
    HAL_GPIO_SetPin(63, HAL_GPIO_PIN_SET);
}

void SysTicks_DelayMS(uint32_t msDelay);
static void IO_Task(void const *argument)
{
    while (1)
    {
        SYS_EVENT_Process();
    }
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
    console_init(0,115200);

    dlog_critical("cpu0 start!!!, time = %s", __TIME__);

    BB_ledGpioInit();
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_BB_EVENT, BB_EventHandler);

    HAL_USB_ConfigPHY();

    HDMI_powerOn();
    
    STRU_HDMI_CONFIGURE        st_configure;

    st_configure.e_getFormatMethod = HAL_HDMI_POLLING;
    st_configure.st_interruptGpio.e_interruptGpioNum = HAL_GPIO_NUM64;
    st_configure.st_interruptGpio.e_interruptGpioPolarity = HAL_GPIO_ACTIVE_HIGH;
    st_configure.st_interruptGpio.e_interruptGpioType = HAL_GPIO_EDGE_SENUMSITIVE;
  
    st_configure.u8_hdmiToEncoderCh = 1;
    HAL_HDMI_RX_Init(HAL_HDMI_RX_0, &st_configure);

    st_configure.st_interruptGpio.e_interruptGpioNum = HAL_GPIO_NUM65;
    st_configure.u8_hdmiToEncoderCh = 0;
    HAL_HDMI_RX_Init(HAL_HDMI_RX_1, &st_configure);

    HAL_USB_InitOTG(HAL_USB_PORT_0);

    HAL_USB_InitOTG(HAL_USB_PORT_1);

    HAL_NV_Init();

	HAL_DMA_init();
/*
	HAL_SD_Init();

	HAL_SD_Fatfs_Init();
*/

    portDISABLE_INTERRUPTS();

    /* Create Main Task */
    osThreadDef(USBMAIN_Task, USB_MainTask, osPriorityBelowNormal, 0, 4 * 128);
    osThreadCreate(osThread(USBMAIN_Task), NULL);

    osThreadDef(USBHStatus_Task, USBH_USBHostStatus, osPriorityNormal, 0, 4 * 128);
    osThreadCreate(osThread(USBHStatus_Task), NULL);

    osThreadDef(IOTask, IO_Task, osPriorityIdle, 0, 16 * 128);
    osThreadCreate(osThread(IOTask), NULL);

    osMessageQDef(osqueue, 1, uint16_t);
    g_usbhAppCtrl.usbhAppEvent  = osMessageCreate(osMessageQ(osqueue),NULL);

    Wireless_TaskInit(WIRELESS_USE_RTOS);

    portENABLE_INTERRUPTS();

    tcpip_init( NULL, NULL );

    osKernelStart();

    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
    }
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
