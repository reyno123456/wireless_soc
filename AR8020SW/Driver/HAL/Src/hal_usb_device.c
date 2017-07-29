/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_usb.c
Description: The external HAL APIs to use the USB Device.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/22
History: 
        0.0.1    2016/12/21    The initial version of hal_usb_device.c
*****************************************************************************/
#include "hal_usb_device.h"
#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_hid.h"
#include "usbd_hid_desc.h"
#include "sram.h"
#include "sys_event.h"
#include "interrupt.h"
#include "hal_nvic.h"
#include "debuglog.h"
#include "hal.h"

USBD_HandleTypeDef          USBD_Device[USBD_PORT_NUM];
USBD_HID_ItfTypeDef         g_stUsbdHidItf;


/**
* @brief  config the usb as device controller
* @param  e_usbPort         usb port number: 0 or 1
* @retval   void
* @note  
*/
void HAL_USB_InitDevice(ENUM_HAL_USB_PORT e_usbPort)
{
    if (HAL_USB_PORT_0 == e_usbPort)
    {
        reg_IrqHandle(OTG_INTR0_VECTOR_NUM, USB_LL_OTG0_IRQHandler, NULL);
        INTR_NVIC_SetIRQPriority(OTG_INTR0_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_OTG_INITR0,0));
    }
    else if (HAL_USB_PORT_1 == e_usbPort)
    {
        reg_IrqHandle(OTG_INTR1_VECTOR_NUM, USB_LL_OTG1_IRQHandler, NULL);
        INTR_NVIC_SetIRQPriority(OTG_INTR1_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_OTG_INITR1,0));
    }
    else
    {
        dlog_error("invalid USB Port Num: %d", e_usbPort);
    }

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_USB_PLUG_OUT, HAL_USB_ResetDevice);

    USBD_Init(&USBD_Device[e_usbPort], &HID_Desc, (uint8_t)e_usbPort);

    USBD_RegisterClass(&USBD_Device[e_usbPort], USBD_HID_CLASS);

    USBD_Start(&USBD_Device[e_usbPort]);

    return;
}


/**
* @brief  reset the usb device controller for usb device hotplug
* @param  void * p         for sys event call-back
* @retval   void
* @note  
*/
void HAL_USB_ResetDevice(void * p)
{
    STRU_SysEvent_DEV_PLUG_OUT  *stDevPlugOut;
    uint8_t                      u8_portId;

    if (p == NULL)
    {
        dlog_error("NULL Pointer");

        return;
    }

    stDevPlugOut    = (STRU_SysEvent_DEV_PLUG_OUT *)p;

    u8_portId       = stDevPlugOut->otg_port_id;

    if (u8_portId > HAL_USB_PORT_NUM)
    {
        dlog_error("Invalid USB PORT Number");
    }

    dlog_info("reset USB%d", u8_portId);

    USBD_LL_Init(&USBD_Device[u8_portId]);
    USBD_LL_Start(&USBD_Device[u8_portId]);

    if (sramReady0 == 1)
    {
        SRAM_Ready0Confirm();
    }
    if (sramReady1 == 1)
    {
        SRAM_Ready1Confirm();
    }

    return;
}


/**
* @brief  send control data to the host
* @param  uint8_t    *buff                the buffer to send
*               uint32_t    u32_len          buffer length 
* @retval   void
* @note
*/
HAL_RET_T HAL_USB_DeviceSendCtrl(uint8_t *buff, uint32_t u32_len, uint8_t u8_portId)
{
    uint8_t         ret;

    ret = USBD_HID_SendReport(&USBD_Device[u8_portId], buff, u32_len, HID_EPIN_CTRL_ADDR);

    if (ret != USBD_OK)
    {
        if (USBD_BUSY == ret)
        {
            dlog_error("send ctrl busy, port: %d", u8_portId);

            return HAL_USB_ERR_DEVICE_BUSY;
        }
        else
        {
            dlog_error("send ctrl not configured, port: %d", u8_portId);

            return HAL_USB_ERR_DEVICE_NOT_CONGIURED;
        }
    }

    return HAL_OK;
}


/**
* @brief   register the user callback function to receive host data
* @param  void (*pUsrFunc)(void *)     user callback function
* @retval   void
* @note
*/
void HAL_USB_RegisterUserProcess(void (*pUsrFunc)(void *, uint8_t),
                               void (*pInitFunc)(void))
{
    g_stUsbdHidItf.dataOut  = pUsrFunc;
    g_stUsbdHidItf.userInit = pInitFunc;

    USBD_HID_RegisterInterface(&USBD_Device[HAL_USB_PORT_0], &g_stUsbdHidItf);
    USBD_HID_RegisterInterface(&USBD_Device[HAL_USB_PORT_1], &g_stUsbdHidItf);
}


uint8_t HAL_USB_DeviceGetConnState(uint8_t u8_usbPortId)
{
    USBD_HandleTypeDef     *pdev;

    pdev                    = &USBD_Device[u8_usbPortId];

    return pdev->u8_connState;
}


/**
* @brief  open the video output to PC or PAD.
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_OpenVideo(uint8_t u8_usbPortId)
{
    USBD_HandleTypeDef      *pdev;

    pdev                    = &USBD_Device[u8_usbPortId];

    USBD_HID_OpenVideoDisplay(pdev);
}


/**
* @brief  close the video output to PC or PAD.
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_CloseVideo(uint8_t u8_usbPortId)
{
    USBD_HandleTypeDef      *pdev;

    pdev                    = &USBD_Device[u8_usbPortId];

    USBD_HID_CloseVideoDisplay(pdev);
}


/**
* @brief  register customer's receive data function
* @param  void (*customerRecv)(void *)
* @retval   void
* @note  
*/
void HAL_USB_RegisterCustomerRecvData(void (*customerRecv)(void *, uint32_t, uint8_t))
{
    g_stUsbdHidItf.customerOut  = customerRecv;

    USBD_HID_RegisterInterface(&USBD_Device[HAL_USB_PORT_0], &g_stUsbdHidItf);
    USBD_HID_RegisterInterface(&USBD_Device[HAL_USB_PORT_1], &g_stUsbdHidItf);
}


/**
* @brief  Customer call this function to send data to host
* @param  uint8_t *buff             customer's data buffer to send
*               uint32_t u32_len       the length of buffer
* @retval   void
* @note  
*/
HAL_RET_T HAL_USB_CustomerSendData(uint8_t *buff,
                                   uint32_t u32_len,
                                   uint8_t u8_portId)
{
    uint8_t             ret;

    ret = USBD_HID_SendReport(&USBD_Device[u8_portId], buff, u32_len, HID_CUSTOMER_IN_ADDR);

    if (ret != USBD_OK)
    {
        if (USBD_BUSY == ret)
        {
            dlog_error("CUSTOMER IN EP BUSY");

            return HAL_USB_ERR_DEVICE_BUSY;
        }
        else
        {
            dlog_error("CUSTOMER IN EP NOT CONFIGURED");

            return HAL_USB_ERR_DEVICE_NOT_CONGIURED;
        }
    }

    return HAL_OK;
}


HAL_RET_T HAL_USB_SendData(uint8_t *buff, uint32_t data_len, uint8_t portId, uint8_t ep)
{
    USBD_HID_HandleTypeDef     *hhid = (USBD_HID_HandleTypeDef*)USBD_Device[portId].pClassData;
    uint32_t                    timeout = 0;

    while (hhid->state[0x7F & ep] != HID_IDLE)
    {
        timeout++;
        HAL_Delay(1);

        /* 50ms timeout */
        if (timeout >= 50)
        {
            dlog_error("usb device send data timeout");
            return HAL_USB_ERR_DEVICE_TIMEOUT;
        }
    }

    hhid->state[0x7F & ep] = HID_BUSY;

    USBD_LL_Transmit (&USBD_Device[portId],
                      ep,
                      buff,
                      data_len);

    return HAL_OK;
}


HAL_RET_T HAL_USB_AudioDataSend(uint8_t *buff, uint16_t audioLen, uint8_t u8_portId)
{
    uint8_t                     ret;
    uint8_t                     retry = 0;
    USBD_HID_HandleTypeDef     *hhid = (USBD_HID_HandleTypeDef*)USBD_Device[u8_portId].pClassData;

    if (USBD_Device[u8_portId].u8_videoDisplay == 0)
    {
        return HAL_OK;
    }

    if (USBD_Device[u8_portId].dev_state != USBD_STATE_CONFIGURED)
    {
        return HAL_OK;
    }

    while (hhid->state[0x7F & HID_EPIN_AUDIO_ADDR]  != HID_IDLE)
    {
        retry++;

        HAL_Delay(1);

        if (retry >= 100)
        {
            dlog_error("audio device busy: %d", hhid->state[0x7F & HID_EPIN_AUDIO_ADDR]);

            return HAL_USB_ERR_DEVICE_BUSY;
        }
    }

    hhid->state[0x7F & HID_EPIN_AUDIO_ADDR]  = HID_BUSY;

    USBD_LL_Transmit (&USBD_Device[u8_portId], HID_EPIN_AUDIO_ADDR, buff, audioLen);

    return HAL_OK;
}
