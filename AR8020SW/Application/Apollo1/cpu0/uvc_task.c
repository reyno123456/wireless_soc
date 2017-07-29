#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "uvc_task.h"
#include "test_usbh.h"
#include "hal_usb_host.h"
#include "hal_usb_device.h"
#include "hal_sram.h"
#include "debuglog.h"
#include "hal.h"


uint8_t                         g_u8ViewUVC = 0;
volatile uint8_t                g_u8SaveUVC = 0;
volatile uint8_t                g_u8ShowUVC = 0;

uint8_t                         u8_FrameBuff[153600];
USBH_UVC_TASK_STATE             g_eUVCTaskState = USBH_UVC_TASK_IDLE;
uint16_t                        g_u16UVCWidth = 0;
uint16_t                        g_u16UVCHeight = 0;
ENUM_HAL_USB_UVC_DATA_TYPE      g_eUVCDataType;
volatile uint8_t                g_u8UserSelectPixel = 0;
FIL                             uvcFile;
uint8_t                         g_u8UVCHeader[12];


void USBH_UVCTask(void const *argument)
{
    dlog_info("UVC Task");

    //USBH_UVCInit(1280, 720, ENUM_UVC_DATA_H264);
    USBH_UVCInit(320, 240, ENUM_UVC_DATA_YUV);

    while (1)
    {
        USBH_ProcUVC();

        HAL_Delay(1);
    }
}


void USBH_UVCInit(uint16_t u16_width, uint16_t u16_height, ENUM_HAL_USB_UVC_DATA_TYPE e_data_type)
{
    g_u16UVCWidth       = u16_width;
    g_u16UVCHeight      = u16_height;
    g_eUVCDataType      = e_data_type;
}


void USBH_ProcUVC(void)
{
    static STRU_UVC_SUPPORT_FORMAT_LIST     stVideoFrameFormat;
    uint32_t                                u32_uvcFrameNum;
    static uint32_t                         u32_UVCFrameSize;
    static uint8_t                          u8_UVCFrameIndex;
    uint8_t                                 i;
    static uint8_t                          u8_uvcPortId;
    static uint8_t                          u8_udiskPortId = HAL_USB_PORT_NUM;
    uint32_t                                u32_savedSize;
    uint8_t                                *u8_buff;
    uint8_t                                 u8_buffCount;
    uint32_t                                u32_lastPacketLen;

    if ((HAL_USB_GetHostAppState(u8_uvcPortId) == HAL_USB_HOST_STATE_DISCONNECT)&&
        (g_eUVCTaskState != USBH_UVC_TASK_IDLE))
    {
        g_eUVCTaskState = USBH_UVC_TASK_DISCONNECT;
    }

    switch (g_eUVCTaskState)
    {
    case USBH_UVC_TASK_IDLE:
        u8_uvcPortId = HAL_USB_GetUVCPortId();

        if (u8_uvcPortId >= HAL_USB_PORT_NUM)
        {
            return;
        }

        if (HAL_USB_HOST_STATE_READY == HAL_USB_GetHostAppState(u8_uvcPortId))
        {
            memset((void *)&stVideoFrameFormat, 0, sizeof(STRU_UVC_SUPPORT_FORMAT_LIST));

            // get supported formats first
            HAL_USB_UVCGetVideoFormats(&stVideoFrameFormat);

            for (i = 0; i < stVideoFrameFormat.u16_frameNum; i++)
            {
                if ((stVideoFrameFormat.st_uvcFrameFormat[i].u16_height == g_u16UVCHeight)&&
                    (stVideoFrameFormat.st_uvcFrameFormat[i].u16_width == g_u16UVCWidth)&&
                    (stVideoFrameFormat.st_uvcFrameFormat[i].e_dataType == g_eUVCDataType))
                {
                    break;
                }
            }

            if (i < stVideoFrameFormat.u16_frameNum)
            {
                g_eUVCTaskState = USBH_UVC_TASK_START;
            }
            else
            {
                dlog_error("no this format: %d * %d", g_u16UVCWidth, g_u16UVCHeight);
            }
        }

        break;

    case USBH_UVC_TASK_START:
        if (HAL_OK == HAL_USB_StartUVC(g_u16UVCWidth,
                                       g_u16UVCHeight,
                                       &u32_UVCFrameSize,
                                       g_eUVCDataType,
                                       u8_uvcPortId))
        {
            dlog_info("start UVC OK!");
        }
        else
        {
            dlog_error("app start UVC fail");

            return;
        }

        g_eUVCTaskState = USBH_UVC_TASK_GET_FRAME;

        break;

    case USBH_UVC_TASK_GET_FRAME:
        if (HAL_OK == HAL_USB_UVCGetVideoFrame(u8_FrameBuff))
        {
            g_eUVCTaskState = USBH_UVC_TASK_CHECK_FRAME_READY;
        }

        break;

    case USBH_UVC_TASK_CHECK_FRAME_READY:
        if (HAL_OK == HAL_USB_UVCCheckFrameReady(&u32_uvcFrameNum, &u32_UVCFrameSize))
        {
            g_eUVCTaskState = USBH_UVC_TASK_GET_FRAME;

            // do something USER need, such as transfer to ground , or optical flow process
            if (g_u8ViewUVC == 1)
            {
                HAL_USB_TransferUVCToGrd(u8_FrameBuff, u32_UVCFrameSize, g_u16UVCWidth, g_u16UVCHeight, ENUM_UVC_DATA_YUV);
            }

            if (g_u8SaveUVC == 1)
            {
                if (u8_udiskPortId >= HAL_USB_PORT_NUM)
                {
                    u8_udiskPortId = HAL_USB_GetMSCPort();

                    if (u8_udiskPortId >= HAL_USB_PORT_NUM)
                    {
                        dlog_error("udisk is not ready to save UVC data");

                        break;
                    }
                }

                f_write(&uvcFile, u8_FrameBuff, u32_UVCFrameSize, (void *)&u32_savedSize);

                if (u32_savedSize < u32_UVCFrameSize)
                {
                    dlog_error("save UVC data error: %d, %d", u32_savedSize, u32_UVCFrameSize);
                }
            }

            if (g_u8ShowUVC == 1)
            {
                /* usb transfer 8K Byte */
                u8_buffCount        = (u32_UVCFrameSize >> 13);
                u32_lastPacketLen   = (u32_UVCFrameSize & (UVC_TRANSFER_SIZE_ONCE - 1));

                // set header of image
                // header format:
                // byte0 byte1 byte2 byte3 byte4 byte5 byte6 byte7 byte8   byte9   byte10 byte11
                // 0x00  0x00  0x00  0x00  0xFF  0xFF  0xFF  0xFF  format  pixel     reserved
                // format:  1: YUV       2: Y only
                // pixel:   1: 160*120   2: 320*240
                g_u8UVCHeader[0]            = 0x00;
                g_u8UVCHeader[1]            = 0x00;
                g_u8UVCHeader[2]            = 0x00;
                g_u8UVCHeader[3]            = 0x00;
                g_u8UVCHeader[4]            = 0xFF;
                g_u8UVCHeader[5]            = 0xFF;
                g_u8UVCHeader[6]            = 0xFF;
                g_u8UVCHeader[7]            = 0xFF;
                g_u8UVCHeader[8]            = 0x01;
                g_u8UVCHeader[9]            = 0x02;
                g_u8UVCHeader[10]           = 0x00;
                g_u8UVCHeader[11]           = 0x00;

                HAL_USB_SendData(g_u8UVCHeader, sizeof(g_u8UVCHeader), (HAL_USB_PORT_1 - u8_uvcPortId), UVC_ENDPOINT_FOR_TRANSFER);

                u8_buff = u8_FrameBuff;

                for (i = 0; i < u8_buffCount; i++)
                {
                    HAL_USB_SendData(u8_buff, UVC_TRANSFER_SIZE_ONCE, (HAL_USB_PORT_1 - u8_uvcPortId), UVC_ENDPOINT_FOR_TRANSFER);

                    u8_buff += UVC_TRANSFER_SIZE_ONCE;
                }

                if (u32_lastPacketLen > 0)
                {
                    HAL_USB_SendData(u8_buff, u32_lastPacketLen, (HAL_USB_PORT_1 - u8_uvcPortId), UVC_ENDPOINT_FOR_TRANSFER);
                }
            }
        }

        break;

    case USBH_UVC_TASK_DISCONNECT:

        g_eUVCTaskState = USBH_UVC_TASK_IDLE;

        break;
    }

}


void command_ViewUVC(void)
{
    HAL_SRAM_EnableSkyBypassVideo(HAL_SRAM_VIDEO_CHANNEL_1);

    g_u8ViewUVC = 1;
}


void command_startUVC(char *width, char *height)
{
    uint16_t                        u16_width   = (uint16_t)strtoul(width, NULL, 0);
    uint16_t                        u16_height  = (uint16_t)strtoul(height, NULL, 0);

    g_eUVCTaskState     = USBH_UVC_TASK_DISCONNECT;

    USBH_UVCInit(u16_width, u16_height, ENUM_UVC_DATA_YUV);
}


void command_saveUVC(void)
{
    FRESULT fileResult;

    USBH_MountUSBDisk();

    fileResult = f_open(&uvcFile, "0:uvcdata.yuv", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);

    if (fileResult != FR_OK)
    {
        dlog_error("create uvc file error: %d", fileResult);
        return;
    }

    dlog_info("start to save UVC data");

    g_u8SaveUVC = 1;
}


void command_stopSaveUVC(void)
{
    f_close(&uvcFile);

    dlog_info("save uvc data succeed");

    g_u8SaveUVC = 0;
}


void command_showUVC(void)
{
    g_u8ShowUVC++;

    if (g_u8ShowUVC >= 2)
    {
        g_u8ShowUVC = 0;
    }
}


void command_getUVCAttribute(char *index, char *type)
{
    uint8_t     uvc_attr_index = 0;
    uint8_t     uvc_attr_type = 0;
    int32_t     uvc_attr_value = 0;

    uvc_attr_index = (uint8_t)strtoul(index, NULL, 0);
    uvc_attr_type = (uint8_t)strtoul(type, NULL, 16);

    if ((uvc_attr_index >= ENUM_HAL_UVC_MAX_NUM) ||
        (uvc_attr_type < HAL_UVC_GET_CUR)||
        (uvc_attr_type > HAL_UVC_GET_DEF))
    {
        dlog_error("invalid attribute: %d");

        command_uvchelp();

        return;
    }

    if (HAL_OK == HAL_USB_HOST_GetUVCAttr(uvc_attr_index, uvc_attr_type, &uvc_attr_value))
    {
        dlog_info("attr_value: %d", uvc_attr_value);
    }
    else
    {
        dlog_error("get attribution is not supported");
    }

    return;
}


void command_setUVCAttribute(char *index, char *value)
{
    uint8_t     uvc_attr_index = 0;
    int32_t     uvc_attr_value = 0;

    uvc_attr_index = (uint8_t)strtol(index, NULL, 0);
    uvc_attr_value = strtol(value, NULL, 0);

    if (uvc_attr_index >= ENUM_HAL_UVC_MAX_NUM)
    {
        dlog_error("invalid attribute");

        command_uvchelp();

        return;
    }

    dlog_info("attr_index: %d, attr_value: %d", uvc_attr_index, uvc_attr_value);

    if (HAL_OK != HAL_USB_HOST_SetUVCAttr(uvc_attr_index, uvc_attr_value))
    {
        dlog_error("set attribution is not supported");
    }

    return;
}


void command_uvchelp(void)
{
    uint32_t     uvc_supported_attr_bitmap;

    uvc_supported_attr_bitmap = HAL_USB_GetUVCProcUnitControls();

    dlog_error("UVC Attribution Usage: setuvcattr <index> <value>");
    dlog_error("UVC Attribution Usage: getuvcattr <index> <type>");

    dlog_error("supported Attribution Bitmap: 0x%08x", uvc_supported_attr_bitmap);

    dlog_error("UVC Attribution Index");
    dlog_error("%d:    BRIGHTNESS", ENUM_HAL_UVC_BRIGHTNESS);
    dlog_error("%d:    CONTRAST", ENUM_HAL_UVC_CONTRAST);
    dlog_error("%d:    HUE", ENUM_HAL_UVC_HUE);
    dlog_error("%d:    SATURATION", ENUM_HAL_UVC_SATURATION);
    dlog_error("%d:    SHARPNESS", ENUM_HAL_UVC_SHARPNESS);
    dlog_error("%d:    GAMMA", ENUM_HAL_UVC_GAMMA);
    dlog_error("%d:    WHITE_BALANCE_TEMP", ENUM_HAL_UVC_WHITE_BALANCE_TEMP);
    dlog_error("%d:    WHITE_BALANCE_COMP", ENUM_HAL_UVC_WHITE_BALANCE_COMP);
    dlog_error("%d:    BACKLIGHT_COMP", ENUM_HAL_UVC_BACKLIGHT_COMP);
    dlog_error("%d:    GAIN", ENUM_HAL_UVC_GAIN);
    dlog_error("%d:   PWR_LINE_FREQ", ENUM_HAL_UVC_PWR_LINE_FREQ);
    dlog_error("%d:   HUE_AUTO", ENUM_HAL_UVC_HUE_AUTO);
    dlog_error("%d:   WHITE_BALANCE_TEMP_AUTO", ENUM_HAL_UVC_WHITE_BALANCE_TEMP_AUTO);
    dlog_error("%d:   WHITE_BALANCE_COMP_AUTO", ENUM_HAL_UVC_WHITE_BALANCE_COMP_AUTO);
    dlog_error("%d:   DIGITAL_MULTI", ENUM_HAL_UVC_DIGITAL_MULTI);
    dlog_error("%d:   DIGITAL_MULTI_LIMIT", ENUM_HAL_UVC_DIGITAL_MULTI_LIMIT);

    dlog_error("UVC Attribution Type");
    dlog_error("0x%02x: GET_CUR", HAL_UVC_GET_CUR);
    dlog_error("0x%02x: GET_MIN", HAL_UVC_GET_MIN);
    dlog_error("0x%02x: GET_MAX", HAL_UVC_GET_MAX);
    dlog_error("0x%02x: GET_RES", HAL_UVC_GET_RES);
    dlog_error("0x%02x: GET_LEN", HAL_UVC_GET_LEN);
    dlog_error("0x%02x: GET_INFO", HAL_UVC_GET_INFO);
    dlog_error("0x%02x: GET_DEF", HAL_UVC_GET_DEF);

    dlog_output(200);
}


