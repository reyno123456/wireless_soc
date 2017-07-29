#include "hal_usb_otg.h"
#include "hal_usb_device.h"
#include "hal_usb_host.h"
#include "sys_event.h"
#include "debuglog.h"
#include "stm32f7xx_ll_usb.h"


/**
* @brief  the callback of otg switch interrupt
* @param  void* p       param for otg switch callback
* @retval   void
* @note  
*/
static void HAL_USB_SwitchOTGCallback(void* p)
{
    STRU_SysEvent_OTG_HOST_DEV_SWITCH *stOTGHostDevSwitch;

    stOTGHostDevSwitch      = (STRU_SysEvent_OTG_HOST_DEV_SWITCH *)p;

    /* switch to host */
    if (stOTGHostDevSwitch->otg_state == 1)
    {
        dlog_info("switch to host");

        HAL_USB_InitHost((ENUM_HAL_USB_PORT)stOTGHostDevSwitch->otg_port_id);
    }
    /* switch to device */
    else if (stOTGHostDevSwitch->otg_state == 0)
    {
        dlog_info("switch to device");

        HAL_USB_InitDevice((ENUM_HAL_USB_PORT)stOTGHostDevSwitch->otg_port_id);
    }
    else
    {
        dlog_error("otg switch state error");
    }
}


/**
* @brief    initiate the USB Port
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_InitOTG(ENUM_HAL_USB_PORT e_usbPort)
{
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_USB_SWITCH_HOST_DEVICE, HAL_USB_SwitchOTGCallback);

    if (e_usbPort == HAL_USB_PORT_0)
    {
        if (USB_LL_GetCurrentOTGIDStatus(USB_OTG0_HS))
        {
            HAL_USB_InitDevice(HAL_USB_PORT_0);
        }
        else
        {
            HAL_USB_InitHost(HAL_USB_PORT_0);
        }
    }
    else
    {
        if (USB_LL_GetCurrentOTGIDStatus(USB_OTG1_HS))
        {
            HAL_USB_InitDevice(HAL_USB_PORT_1);
        }
        else
        {
            HAL_USB_InitHost(HAL_USB_PORT_1);
        }
    }
    
}


/**
* @brief    Configure the parameters optimized by IC Designer
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_ConfigPHY(void)
{
    USB_LL_ConfigPhy();
}


