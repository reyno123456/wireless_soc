/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_usb_host.h
Description: The external HAL APIs to use the SRAM.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/22
History: 
        0.0.1    2016/12/22    The initial version of hal_usb_host.h
*****************************************************************************/

#ifndef __HAL_USB_HOST_H__
#define __HAL_USB_HOST_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "hal_ret_type.h"
#include "hal_usb_otg.h"


#define HAL_USB_UVC_MAX_FRAME_FORMATS_NUM     20

typedef enum
{
    HAL_USB_HOST_STATE_IDLE = 0,
    HAL_USB_HOST_STATE_READY,
    HAL_USB_HOST_STATE_DISCONNECT,
} ENUM_HAL_USB_HOST_STATE;


typedef enum
{
    HAL_USB_HOST_CLASS_MSC = 0,
    HAL_USB_HOST_CLASS_UVC,
    HAL_USB_HOST_CLASS_NONE,
} ENUM_HAL_USB_HOST_CLASS;


typedef enum
{
    HAL_UVC_GET_CUR = 0x81,
    HAL_UVC_GET_MIN = 0x82,
    HAL_UVC_GET_MAX = 0x83,
    HAL_UVC_GET_RES = 0x84,
    HAL_UVC_GET_LEN = 0x85,
    HAL_UVC_GET_INFO = 0x86,
    HAL_UVC_GET_DEF = 0x87,
} ENUM_HAL_UVC_GET_ATTRIBUTE_TYPE;


typedef enum
{
    ENUM_HAL_UVC_BRIGHTNESS                = 0,
    ENUM_HAL_UVC_CONTRAST                  = 1,
    ENUM_HAL_UVC_HUE                       = 2,
    ENUM_HAL_UVC_SATURATION                = 3,
    ENUM_HAL_UVC_SHARPNESS                 = 4,
    ENUM_HAL_UVC_GAMMA                     = 5,
    ENUM_HAL_UVC_WHITE_BALANCE_TEMP        = 6,
    ENUM_HAL_UVC_WHITE_BALANCE_COMP        = 7,
    ENUM_HAL_UVC_BACKLIGHT_COMP            = 8,
    ENUM_HAL_UVC_GAIN                      = 9,
    ENUM_HAL_UVC_PWR_LINE_FREQ             = 10,
    ENUM_HAL_UVC_HUE_AUTO                  = 11,
    ENUM_HAL_UVC_WHITE_BALANCE_TEMP_AUTO   = 12,
    ENUM_HAL_UVC_WHITE_BALANCE_COMP_AUTO   = 13,
    ENUM_HAL_UVC_DIGITAL_MULTI             = 14,
    ENUM_HAL_UVC_DIGITAL_MULTI_LIMIT       = 15,
    ENUM_HAL_UVC_MAX_NUM                   = 16,
} ENUM_HAL_UVC_ATTRIBUTE_INDEX;


typedef enum
{
    ENUM_UVC_DATA_YUV   = 1,
    ENUM_UVC_DATA_Y     = 2,
    ENUM_UVC_DATA_H264  = 3,
} ENUM_HAL_USB_UVC_DATA_TYPE;


typedef struct
{
    uint16_t    u16_width;
    uint16_t    u16_height;
    ENUM_HAL_USB_UVC_DATA_TYPE e_dataType;
} STRU_UVC_VIDEO_FRAME_FORMAT;


typedef struct
{
    uint16_t    u16_frameNum;
    STRU_UVC_VIDEO_FRAME_FORMAT st_uvcFrameFormat[HAL_USB_UVC_MAX_FRAME_FORMATS_NUM];
} STRU_UVC_SUPPORT_FORMAT_LIST;



/**
* @brief  Set the USB Host State for Application use.
* @param  e_usbHostAppState             indicate the usb host state
* @retval   void
* @note  
*/
void HAL_USB_SetHostAppState(ENUM_HAL_USB_HOST_STATE e_usbHostAppState, uint8_t port_id);

/**
* @brief  Get the USB Host State for Application use.
* @param  void
* @retval   HAL_USB_STATE_IDLE                indicate the usb is IDLE
*               HAL_USB_STATE_READY             indicate the usb is READY
*               HAL_USB_STATE_DISCONNECT   indicate the usb is DISCONNECT
* @note  
*/
ENUM_HAL_USB_HOST_STATE HAL_USB_GetHostAppState(uint8_t port_id);

/**
* @brief  polling the usb state-machine 
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_HostProcess(void);

/**
* @brief  config the usb as host controller
* @param  e_usbPort            usb port number: 0 or 1
*               e_usbHostClass    usb class, MSC or UVC
* @retval   void
* @note  
*/
void HAL_USB_InitHost(ENUM_HAL_USB_PORT e_usbPort);

/**
* @brief  start the USB Video for Application use
* @param  uint16_t u16_width                  user input frame width
*               uint16_t u16_height                 user input frame height
*               uint32_t *u32_frameSize          return the frame size
* @retval   void
* @note  
*/
HAL_RET_T HAL_USB_StartUVC(uint16_t u16_width,
                           uint16_t u16_height,
                           uint32_t *u32_frameSize,
                           ENUM_HAL_USB_UVC_DATA_TYPE e_UVCDataType,
                           uint8_t u8_uvcPortId);

/**
* @brief  get the latest frame buffer
* @param  uint8_t  *u8_buff    the dest buffer to storage the video frame
* @retval   HAL_USB_ERR_BUFF_IS_EMPTY   : means the buffer pool is empty
*               HAL_OK                                      : means successfully get one video frame
* @note  
*/
HAL_RET_T HAL_USB_UVCGetVideoFrame(uint8_t *u8_buff);


/**
* @brief  get the latest frame buffer
* @param  *u32_frameNum           frame number in frame serises
*               *u32_frameSize            data size if  a frame 
* @retval   HAL_OK                       frame is ready
* @note  
*/
HAL_RET_T HAL_USB_UVCCheckFrameReady(uint32_t *u32_frameNum,
                                     uint32_t *u32_frameSize);

/**
* @brief  get the formats this camera support
* @param  
* @retval   STRU_UVC_SUPPORT_FORMAT_LIST *stVideoFrameFormat
* @note  
*/
void HAL_USB_UVCGetVideoFormats(STRU_UVC_SUPPORT_FORMAT_LIST *stVideoFrameFormat);


/**
* @brief  get the UVC Porc Unit Control Mask, such as HUE,Backlight, white balance and so on
* @param  void
* @retval  void
* @note  
*/
uint32_t HAL_USB_GetUVCProcUnitControls(void);


uint32_t HAL_USB_GetUVCExtUnitControls(void);


/**
* @brief    configure the USB Controller to enter into TEST MODE
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_EnterUSBHostTestMode(void);


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
                             ENUM_HAL_USB_UVC_DATA_TYPE e_UVCDataType);

ENUM_HAL_USB_HOST_CLASS HAL_USB_CurUsbClassType(uint8_t port_id);

uint8_t HAL_USB_GetUVCPortId(void);

uint8_t HAL_USB_GetMSCPort(void);

HAL_RET_T HAL_USB_HOST_SetUVCAttr(ENUM_HAL_UVC_ATTRIBUTE_INDEX uvc_attr_index,
                                  int32_t uvc_attr_value);

HAL_RET_T HAL_USB_HOST_GetUVCAttr(ENUM_HAL_UVC_ATTRIBUTE_INDEX uvc_attr_index,
                                ENUM_HAL_UVC_GET_ATTRIBUTE_TYPE uvc_attr_type,
                                int32_t *uvc_attr_value);


#ifdef __cplusplus
}
#endif

#endif

