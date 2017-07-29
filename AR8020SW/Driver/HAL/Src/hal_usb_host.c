/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_usb.c
Description: The external HAL APIs to use the USB Host.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/22
History: 
        0.0.1    2016/12/21    The initial version of hal_usb_host.c
*****************************************************************************/
#include "hal_usb_host.h"
#include "usbh_def.h"
#include "usbh_core.h"
#include "usbh_uvc.h"
#include "usbh_msc.h"
#include "interrupt.h"
#include "hal_nvic.h"
#include "debuglog.h"
#include "hal_dma.h"
#include "cpu_info.h"
#include "systicks.h"
#include "hal.h"
#include "sram.h"

static ENUM_HAL_USB_HOST_STATE   s_eUSBHostAppState[HAL_USB_PORT_NUM];
USBH_HandleTypeDef               hUSBHost[USBH_PORT_NUM];
uint8_t                          u8_header[12];


/**
* @brief  Set the USB Host State for Application use.
* @param  e_usbHostAppState             indicate the usb host state
* @retval   void
* @note  
*/
void HAL_USB_SetHostAppState(ENUM_HAL_USB_HOST_STATE e_usbHostAppState, uint8_t port_id)
{
    s_eUSBHostAppState[port_id] = e_usbHostAppState;
}


/**
* @brief  Get the USB Host State for Application use.
* @param  void
* @retval   HAL_USB_STATE_IDLE                indicate the usb is IDLE
*               HAL_USB_STATE_READY             indicate the usb is READY
*               HAL_USB_STATE_DISCONNECT   indicate the usb is DISCONNECT
* @note  
*/
ENUM_HAL_USB_HOST_STATE HAL_USB_GetHostAppState(uint8_t port_id)
{
    return s_eUSBHostAppState[port_id];
}


/**
* @brief  polling the usb state-machine 
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_HostProcess(void)
{
    uint8_t         i;

    for (i = 0; i < HAL_USB_PORT_NUM; i++)
    {
        USBH_Process(&hUSBHost[i]);
    }
}


/**
* @brief  the entrance of usb state change, called when state change happened
* @param  phost       the handler of usb host
*               id            the usb host state
* @retval   void
* @note  
*/
static void USB_HostAppState(USBH_HandleTypeDef *phost, uint8_t id)
{
    switch(id)
    {
        case HOST_USER_SELECT_CONFIGURATION:
            break;

        case HOST_USER_DISCONNECTION:
            HAL_USB_SetHostAppState(HAL_USB_HOST_STATE_DISCONNECT, phost->id);
            break;

        case HOST_USER_CLASS_ACTIVE:
            HAL_USB_SetHostAppState(HAL_USB_HOST_STATE_READY, phost->id);
            break;

        case HOST_USER_CONNECTION:
            break;

        default:
            break;
    }

}


/**
* @brief  config the usb as host controller
* @param  e_usbPort            usb port number: 0 or 1
*               e_usbHostClass    usb class, MSC or UVC
* @retval   void
* @note  
*/
void HAL_USB_InitHost(ENUM_HAL_USB_PORT e_usbPort)
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

    USBH_Init(&hUSBHost[e_usbPort], USB_HostAppState, (uint8_t)e_usbPort);

    //support MSC
    USBH_RegisterClass(&hUSBHost[e_usbPort], USBH_MSC_CLASS);

    //support UVC
    USBH_RegisterClass(&hUSBHost[e_usbPort], USBH_UVC_CLASS);

    USBH_Start(&hUSBHost[e_usbPort]);
}


/**
* @brief  start the USB Video for Application use
* @param  void
* @retval   void
* @note  
*/
HAL_RET_T HAL_USB_StartUVC(uint16_t u16_width,
                           uint16_t u16_height,
                           uint32_t *u32_frameSize,
                           ENUM_HAL_USB_UVC_DATA_TYPE e_UVCDataType,
                           uint8_t u8_uvcPortId)
{
    HAL_RET_T           u8_ret = HAL_OK;
    uint8_t             u8_frameIndex;
    uint8_t             u8_formatIndex;
    uint8_t             i;
    UVC_SupportedFormatsDef uvc_format;
    uint8_t             u8_frameNum;

    if (u8_uvcPortId > HAL_USB_PORT_NUM)
    {
        dlog_error("invalid usb port number");

        return HAL_USB_ERR_USBH_UVC_INVALID_PARAM;
    }

    if ((u16_width == 0)||(u16_height == 0))
    {
        dlog_error("width or height can not be ZERO");

        return HAL_USB_ERR_USBH_UVC_INVALID_PARAM;
    }

    if (e_UVCDataType == ENUM_UVC_DATA_H264)
    {
        uvc_format = UVC_SUPPORTED_FORMAT_FRAME_BASED;

        u8_frameNum = USBH_UVC_GetFrameFrameNum(&hUSBHost[u8_uvcPortId]);
    }
    else
    {
        uvc_format = UVC_SUPPORTED_FORMAT_UNCOMPRESSED;

        u8_frameNum = USBH_UVC_GetFrameUncompNum(&hUSBHost[u8_uvcPortId]);
    }

    for (i = 0; i < u8_frameNum; i++)
    {
        if ((u16_width == USBH_UVC_GetFrameWidth(i, uvc_format))&&
            (u16_height == USBH_UVC_GetFrameHeight(i, uvc_format)))
        {
            break;
        }
    }

    if (i < u8_frameNum)
    {
        u8_frameIndex       = USBH_UVC_GetFrameIndex(i, uvc_format);
        u8_formatIndex      = USBH_UVC_GetFormatIndex(uvc_format);

        if (uvc_format == UVC_SUPPORTED_FORMAT_UNCOMPRESSED)
        {
            *u32_frameSize      = USBH_UVC_GetFrameSize(u8_frameIndex);
        }

        dlog_info("u8_frameIndex, u8_formatIndex: %d, %d", u8_frameIndex, u8_formatIndex);

        if (0 == USBH_UVC_StartView(&hUSBHost[u8_uvcPortId], u8_frameIndex, u8_formatIndex, uvc_format))
        {
            dlog_info("START UVC OK");
        }
        else
        {
            dlog_error("START UVC FAIL");

            u8_ret          = HAL_USB_ERR_USBH_UVC_START_ERROR;
        }
    }
    else
    {
        u8_ret              = HAL_USB_ERR_USBH_UVC_FORMAT_ERROR;
    }

    return u8_ret;
}


/**
* @brief  get the latest frame buffer
* @param  uint8_t  *u8_buff    the dest buffer to storage the video frame
* @retval   HAL_USB_ERR_BUFF_IS_EMPTY   : means the buffer pool is empty
*               HAL_OK                                      : means successfully get one video frame
* @note  
*/
HAL_RET_T HAL_USB_UVCGetVideoFrame(uint8_t *u8_buff)
{
    HAL_RET_T                   ret = HAL_USB_ERR_UVC_LAST_FRAME_PREPARING;

    if (g_stUVCUserInterface.u8_userWaiting != UVC_USER_GET_FRAME_IDLE)
    {
        return ret;
    }

    g_stUVCUserInterface.u8_userWaiting = UVC_USER_GET_FRAME_WAITING;
    g_stUVCUserInterface.u8_userBuffer  = u8_buff;

    ret = HAL_OK;

    return ret;
}


/**
* @brief  get the latest frame buffer
* @param      *u32_frameNum      frame number in the frame series
*                   *u32_frameSize      data size of an frame
* @retval       HAL_RET_T             whether the frame is ready to use
* @note  
*/
HAL_RET_T HAL_USB_UVCCheckFrameReady(uint32_t *u32_frameNum,
                                     uint32_t *u32_frameSize)
{
    if (g_stUVCUserInterface.u8_userWaiting == UVC_USER_GET_FRAME_FINISHED)
    {
        *u32_frameNum       = g_stUVCUserInterface.u32_frameIndex;
        *u32_frameSize      = g_stUVCUserInterface.u32_frameLen;

        g_stUVCUserInterface.u8_userWaiting = UVC_USER_GET_FRAME_IDLE;

        return HAL_OK;
    }
    else
    {
        return HAL_USB_ERR_USBH_UVC_FRAME_NOT_READY;
    }
}


/**
* @brief  get formats the camera support
* @param  STRU_UVC_VIDEO_FRAME_FORMAT *stVideoFrameFormat
* @retval   void
* @note  
*/
void HAL_USB_UVCGetVideoFormats(STRU_UVC_SUPPORT_FORMAT_LIST *stVideoFrameFormat)
{
    uint8_t         i;
    uint8_t         j;
    uint8_t         u8_uvcPortId;
    uint8_t         u8_frameNum = 0;

    u8_uvcPortId    = HAL_USB_GetUVCPortId();

    if (u8_uvcPortId > HAL_USB_PORT_NUM)
    {
        dlog_error("invalid usb port number");

        return;
    }

    u8_frameNum = USBH_UVC_GetFrameUncompNum(&hUSBHost[u8_uvcPortId]);

    if (u8_frameNum > 0)
    {
        dlog_info("YUV Formats:");

        for (i = 0; i < u8_frameNum; i++)
        {
            stVideoFrameFormat->st_uvcFrameFormat[i].u16_height = USBH_UVC_GetFrameHeight(i, UVC_SUPPORTED_FORMAT_UNCOMPRESSED);
            stVideoFrameFormat->st_uvcFrameFormat[i].u16_width  = USBH_UVC_GetFrameWidth(i, UVC_SUPPORTED_FORMAT_UNCOMPRESSED);
            stVideoFrameFormat->st_uvcFrameFormat[i].e_dataType = ENUM_UVC_DATA_YUV;

            (stVideoFrameFormat->u16_frameNum)++;

            if (stVideoFrameFormat->u16_frameNum >= HAL_USB_UVC_MAX_FRAME_FORMATS_NUM)
            {
                dlog_error("exceed the max number for frame formats");

                return;
            }

            dlog_info("i: %d, width: %d, height: %d",
                      i,
                      stVideoFrameFormat->st_uvcFrameFormat[i].u16_width,
                      stVideoFrameFormat->st_uvcFrameFormat[i].u16_height);
        }
    }

    u8_frameNum = USBH_UVC_GetFrameFrameNum(&hUSBHost[u8_uvcPortId]);

    if (u8_frameNum > 0)
    {
        dlog_info("H264 Formats:");

        for (j = 0; j < u8_frameNum; j++)
        {
            stVideoFrameFormat->st_uvcFrameFormat[j+i].u16_height = USBH_UVC_GetFrameHeight(j, UVC_SUPPORTED_FORMAT_FRAME_BASED);
            stVideoFrameFormat->st_uvcFrameFormat[j+i].u16_width  = USBH_UVC_GetFrameWidth(j, UVC_SUPPORTED_FORMAT_FRAME_BASED);
            stVideoFrameFormat->st_uvcFrameFormat[j+i].e_dataType = ENUM_UVC_DATA_H264;

            (stVideoFrameFormat->u16_frameNum)++;

            if (stVideoFrameFormat->u16_frameNum >= HAL_USB_UVC_MAX_FRAME_FORMATS_NUM)
            {
                dlog_error("exceed the max number for frame formats");

                return;
            }

            dlog_info("i: %d, width: %d, height: %d",
                      i,
                      stVideoFrameFormat->st_uvcFrameFormat[j+i].u16_width,
                      stVideoFrameFormat->st_uvcFrameFormat[j+i].u16_height);
        }
    }
}


/**
* @brief  get the control items supported by Processing Unit, such as white balance, hue, and so on
* @param  void
* @retval   uint32_t, bit maps
* @note  
*/
uint32_t HAL_USB_GetUVCProcUnitControls(void)
{
    uint32_t        ret;

    ret = USBH_UVC_GetProcUnitControls();

    return ret;
}


/**
* @brief  get the control items supported by Extension Unit
* @param  void
* @retval   uint32_t, bit maps
* @note  
*/
uint32_t HAL_USB_GetUVCExtUnitControls(void)
{
    uint32_t        ret;

    ret = USBH_UVC_GetExtUnitControls();

    return ret;
}


/**
* @brief    configure the USB Controller to enter into TEST MODE
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_EnterUSBHostTestMode(void)
{
    USB_LL_EnterHostTestMode(USB_OTG0_HS);
}


/*
 * @brief    transfer UVC Data To Ground
 * @param  *buff                    UVC Data Buffer
                  dataLen                data size of a UVC frame
                  width                    frame width
                  height                   frame height
                  e_UVCDataType     YUV or Y only
 * @retval   void
 * @note  
 */
void HAL_USB_TransferUVCToGrd(uint8_t *buff,
                             uint32_t dataLen,
                             uint16_t width,
                             uint16_t height,
                             ENUM_HAL_USB_UVC_DATA_TYPE e_UVCDataType)
{
    static uint8_t          u8_frameInterval = 0;
    uint8_t                 u8_frameCounter;
    static uint32_t         u32_systickRecord = 0;

    u8_frameInterval++;

    // to save bandwidth, every u8_frameCounter frames just transfer one frame
    if ((width == 320)&&
        (height == 240))
    {
        if (0 != u32_systickRecord)
        {
            if ((HAL_GetSysMsTick() - u32_systickRecord) <= 80)
            {
                return;
            }
            else
            {
                u32_systickRecord = HAL_GetSysMsTick();
            }
        }
        else
        {
            u32_systickRecord = HAL_GetSysMsTick();
        }
    }
    else
    {
        if (0 != u32_systickRecord)
        {
            if ((HAL_GetSysMsTick() - u32_systickRecord) <= 20)
            {
                return;
            }
            else
            {
                u32_systickRecord = HAL_GetSysMsTick();
            }
        }
        else
        {
            u32_systickRecord = HAL_GetSysMsTick();
        }
    }

    u8_frameInterval    = 0;

    // set header of image
    // header format:
    // byte0 byte1 byte2 byte3 byte4 byte5 byte6 byte7 byte8   byte9   byte10 byte11
    // 0x00  0x00  0x00  0x00  0xFF  0xFF  0xFF  0xFF  format  pixel     reserved
    // format:  1: YUV       2: Y only
    // pixel:   1: 160*120   2: 320*240
    u8_header[0]            = 0x00;
    u8_header[1]            = 0x00;
    u8_header[2]            = 0x00;
    u8_header[3]            = 0x00;
    u8_header[4]            = 0xFF;
    u8_header[5]            = 0xFF;
    u8_header[6]            = 0xFF;
    u8_header[7]            = 0xFF;

    if (e_UVCDataType == ENUM_UVC_DATA_Y)
    {
        u8_header[8]        = 0x02;
    }
    else
    {
        u8_header[8]        = 0x01;
    }

    if ((width == 320) && (height == 240) )
    {
        u8_header[9]        = 0x02;
    }
    else
    {
        u8_header[9]        = 0x01;
    }

    u8_header[10]           = 0x00;
    u8_header[11]           = 0x00;

    // copy header to baseband
    memcpy((void *)VIDEO_BYPASS_CHANNEL_1_DEST_ADDR,
           (void *)u8_header,
           sizeof(u8_header));
    // copy frame data to baseband
    memcpy((void *)(VIDEO_BYPASS_CHANNEL_1_DEST_ADDR + sizeof(u8_header)),
           (void *)buff,
           dataLen);
}


ENUM_HAL_USB_HOST_CLASS HAL_USB_CurUsbClassType(uint8_t port_id)
{
    USBH_ClassTypeDef           *activeClass = NULL;
    ENUM_HAL_USB_HOST_CLASS      enHostClass = HAL_USB_HOST_CLASS_NONE;

    activeClass            = hUSBHost[port_id].pActiveClass;

    if (NULL == activeClass)
    {
        dlog_error("active class is NULL");
        return enHostClass;
    }

    switch (activeClass->ClassCode)
    {
    case USB_MSC_CLASS:
        enHostClass        = HAL_USB_HOST_CLASS_MSC;
        break;

    case UVC_CLASS:
        enHostClass        = HAL_USB_HOST_CLASS_UVC;
        break;

    default:
        enHostClass        = HAL_USB_HOST_CLASS_NONE;
        break;
    }

    return enHostClass;
}


uint8_t HAL_USB_GetUVCPortId(void)
{
    USBH_ClassTypeDef      *activeClass = NULL;
    uint8_t                 u8_uvcPortId = HAL_USB_PORT_NUM;

    if (g_u8UVCPortId > HAL_USB_PORT_NUM)
    {
        return HAL_USB_PORT_NUM;
    }

    activeClass             = hUSBHost[g_u8UVCPortId].pActiveClass;

    if (activeClass != NULL)
    {
        if (activeClass->ClassCode == UVC_CLASS)
        {
            u8_uvcPortId = g_u8UVCPortId;
        }
    }

    return u8_uvcPortId;
}


uint8_t HAL_USB_GetMSCPort(void)
{
    return g_mscPortId;
}


HAL_RET_T HAL_USB_HOST_SetUVCAttr(ENUM_HAL_UVC_ATTRIBUTE_INDEX uvc_attr_index,
                                  int32_t uvc_attr_value)
{
    uint8_t     u8_uvcPortId = HAL_USB_PORT_0;

    u8_uvcPortId = HAL_USB_GetUVCPortId();

    if (u8_uvcPortId >= HAL_USB_PORT_NUM)
    {
        dlog_error("get uvc port error");

        return HAL_USB_ERR_PORT_INVALID;
    }

    if (0 != USBH_UVC_SetUVCAttrInterface(&hUSBHost[u8_uvcPortId], (uint8_t)uvc_attr_index, uvc_attr_value))
    {
        dlog_error("UVC Attribution not suppoprted");

        return HAL_USB_ERR_USBH_UVC_INVALID_PARAM;
    }

    return HAL_OK;
}


HAL_RET_T HAL_USB_HOST_GetUVCAttr(ENUM_HAL_UVC_ATTRIBUTE_INDEX uvc_attr_index,
                                ENUM_HAL_UVC_GET_ATTRIBUTE_TYPE uvc_attr_type,
                                int32_t *uvc_attr_value)
{
    uint8_t     u8_uvcPortId = HAL_USB_PORT_0;
    int8_t      ret;

    u8_uvcPortId = HAL_USB_GetUVCPortId();

    if (u8_uvcPortId >= HAL_USB_PORT_NUM)
    {
        dlog_error("get uvc port error");

        return HAL_USB_ERR_PORT_INVALID;
    }

    ret = USBH_UVC_GetUVCAttrInterface(&hUSBHost[u8_uvcPortId],
                                      (uint8_t)uvc_attr_index,
                                      (uint8_t)uvc_attr_type,
                                      uvc_attr_value);

    if (ret != 0)
    {
        dlog_error("UVC Attribution not suppoprted");

        return HAL_USB_ERR_USBH_UVC_INVALID_PARAM;
    }

    return HAL_OK;
}

