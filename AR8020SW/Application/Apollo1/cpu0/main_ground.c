#include "debuglog.h"
#include "command.h"
#include "serial.h"
#include "hal_sram.h"
#include "cmsis_os.h"
#include "sys_event.h"
#include "bb_spi.h"
#include "test_usbh.h"
#include "com_task.h"
#include "hal.h"
#include "hal_bb.h"
#include "hal_usb_otg.h"
#include "hal_sys_ctl.h"
#include "wireless_interface.h"
#include "hal_nv.h"

void console_init(uint32_t uart_num, uint32_t baut_rate)
{
    dlog_init(command_run, NULL, DLOG_CLIENT_PROCESSOR);
}

static void IO_Task(void const *argument)
{
    while (1)
    {
        HAL_SRAM_CheckChannelTimeout();

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
    pst_cfg->u8_workMode = 1;
    HAL_SYS_CTL_Init(pst_cfg);

    /* initialize the uart */
    console_init(0,115200);
    dlog_info("cpu0 start!!! \n");

    HAL_USB_ConfigPHY();

    HAL_USB_InitOTG(HAL_USB_PORT_0);

    HAL_USB_InitOTG(HAL_USB_PORT_1);

    HAL_SRAM_ReceiveVideoConfig();

    HAL_SRAM_ChannelConfig(ENUM_HAL_SRAM_CHANNEL_TYPE_VIDEO0,
                           HAL_USB_PORT_0,
                           0);

    HAL_SRAM_ChannelConfig(ENUM_HAL_SRAM_CHANNEL_TYPE_VIDEO1,
                           HAL_USB_PORT_1,
                           1);

    HAL_NV_Init();

    portDISABLE_INTERRUPTS();

    osThreadDef(USBHStatus_Task, USBH_USBHostStatus, osPriorityNormal, 0, 4 * 128);
    osThreadCreate(osThread(USBHStatus_Task), NULL);

    osThreadDef(IOTask, IO_Task, osPriorityNormal, 0, 16 * 128);
    osThreadCreate(osThread(IOTask), NULL);

    Wireless_TaskInit(WIRELESS_USE_RTOS);

    COMTASK_Init();

    portENABLE_INTERRUPTS();

    osKernelStart();

    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
