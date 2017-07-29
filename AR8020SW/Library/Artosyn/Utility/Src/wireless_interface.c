#include <string.h>
#include "wireless_interface.h"
#include "hal_usb_device.h"
#include "debuglog.h"
#include "hal_bb.h"
#include "hal_sram.h"
#include "bb_types.h"
#include "cmsis_os.h"
#include "md5.h"
#include "nor_flash.h"
#include "hal.h"

STRU_WIRELESS_INFO_DISPLAY             *g_pstWirelessInfoDisplay;        //OSD Info in SRAM
STRU_WIRELESS_INFO_DISPLAY              g_stWirelessInfoSend;            //send OSD to PAD or PC
STRU_WIRELESS_MESSAGE_BUFF              g_stWirelessParamConfig;     //receive from PAD or PC
STRU_WIRELESS_MESSAGE_BUFF              g_stWirelessReply;           //send to PAD or PC

uint8_t eventFlag = 0;

volatile uint8_t                        g_u8OSDToggle = 0;
volatile uint8_t                        g_u8OSDEnable[HAL_USB_PORT_NUM] = {0, 0};

WIRELESS_CONFIG_HANDLER g_stWirelessMsgHandler[MAX_PID_NUM] = 
{
    NULL,
    WIRELESS_INTERFACE_UPGRADE_Handler,
    WIRELESS_INTERFACE_ENTER_TEST_MODE_Handler,
    WIRELESS_INTERFACE_SYNCHRONIZE_FREQ_CHANNEL_Handler,
    WIRELESS_INTERFACE_AUTO_MODE_Handler,
    WIRELESS_INTERFACE_SELF_ADAPTION_MODE_Handler,
    WIRELESS_INTERFACE_SWITCH_TX_MODE_Handler,
    WIRELESS_INTERFACE_SWITCH_RX_MODE_Handler,
    WIRELESS_INTERFACE_SWITCH_USB1_MODE_Handler,
    WIRELESS_INTERFACE_SWITCH_USB2_MODE_Handler,
    WIRELESS_INTERFACE_ALL_RESET_Handler,
    WIRELESS_INTERFACE_RX_RESET_Handler,
    WIRELESS_INTERFACE_TX_RESET_Handler,
    WIRELESS_INTERFACE_MIMO_1T2R_Handler,
    WIRELESS_INTERFACE_WRITE_BB_REG_Handler,
    WIRELESS_INTERFACE_READ_BB_REG_Handler,
    WIRELESS_INTERFACE_MIMO_2T2R_Handler,
    WIRELESS_INTERFACE_OSD_DISPLAY_Handler,
    NULL,
    WIRELESS_INTERFACE_GET_ID_Handler,
    WIRELESS_INTERFACE_GET_GROUND_TX_PWR_Handler,
    WIRELESS_INTERFACE_GET_SKY_TX_PWR_Handler,
    WIRELESS_INTERFACE_GET_RC_FREQ_CHANNEL_Handler,
    WIRELESS_INTERFACE_GET_VIDEO_FREQ_CHANNEL_Handler,
    WIRELESS_INTERFACE_GET_SOFTWARE_VERSION_Handler,
    WIRELESS_INTERFACE_GET_DEV_INFO_Handler,
    WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_MASK_Handler,
    WIRELESS_INTERFACE_SET_VIDEO_FREQ_CHANNEL_MASK_Handler,
    WIRELESS_INTERFACE_SELECT_VIDEO_FREQ_CHANNEL_Handler,
    WIRELESS_INTERFACE_SWITCH_QAM_MODE_Handler,
    WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_Handler,
    WIRELESS_INTERFACE_SET_TX_PWR_Handler,
    WIRELESS_INTERFACE_SET_VIDEO_TX_ID_Handler,
    WIRELESS_INTERFACE_SET_CTRL_ID_Handler,
    WIRELESS_INTERFACE_SWITCH_WORKING_FREQ_BAND_Handler,
    WIRELESS_INTERFACE_RC_SCAN_ALL_BAND_Handler,
    WIRELESS_INTERFACE_RC_SCAN_WORKING_BAND_Handler,
    WIRELESS_INTERFACE_VIDEO_SCAN_ALL_BAND_Handler,
    WIRELESS_INTERFACE_VIDEO_SCAN_WORKING_BAND_Handler,
    WIRELESS_INTERFACE_RECOVER_TO_FACTORY_Handler,
    WIRELESS_INTERFACE_RC_HOPPING_Handler,
    WIRELESS_INTERFACE_SAVE_CONFIGURE_Handler,
    WIRELESS_INTERFACE_READ_MCU_ID_Handler,       
    WIRELESS_INTERFACE_LOAD_SKY_REG_TABLE_Handler,
    WIRELESS_INTERFACE_LOAD_GRD_REG_TABLE_Handler,
    WIRELESS_INTERFACE_SWITCH_BB_POWER_Handler,   
    WIRELESS_INTERFACE_SKY_ONLY_RX_Handler,       
    WIRELESS_INTERFACE_SWITCH_RF_PWR_0_Handler,   
    WIRELESS_INTERFACE_SWITCH_RF_PWR_1_Handler,   
    WIRELESS_INTERFACE_EXT_ONEKEY_IT_Handler,     
    WIRELESS_INTERFACE_SWITCH_IT_CHAN_Handler,    
    WIRELESS_INTERFACE_SWITCH_RMT_CHAN_Handler,   
    WIRELESS_INTERFACE_SET_PWR_CAL_0_Handler,     
    WIRELESS_INTERFACE_SET_PWR_CAL_1_Handler,     
    WIRELESS_INTERFACE_RST_MCU_Handler,           
    WIRELESS_INTERFACE_RF_PWR_AUTO_Handler,       
    WIRELESS_INTERFACE_SWITCH_DEBUG_MODE_Handler, 
    WIRELESS_INTERFACE_READ_RF_REG_Handler,
    WIRELESS_INTERFACE_WRITE_RF_REG_Handler,
    WIRELESS_INTERFACE_OPEN_ADAPTION_BIT_STREAM_Handler,
    WIRELESS_INTERFACE_SWITCH_CH1_Handler,
    WIRELESS_INTERFACE_SWITCH_CH2_Handler,
    WIRELESS_INTERFACE_SET_CH1_BIT_RATE_Handler,
    WIRELESS_INTERFACE_SET_CH2_BIT_RATE_Handler,
    WIRELESS_INTERFACE_VIDEO_QAM_Handler,
    WIRELESS_INTERFACE_VIDEO_CODE_RATE_Handler,
    WIRELESS_INTERFACE_RC_QAM_Handler,
    WIRELESS_INTERFACE_RC_CODE_RATE_Handler,
    WIRELESS_INTERFACE_OPEN_VIDEO_Handler,
    WIRELESS_INTERFACE_CLOSE_VIDEO_Handler,
    WIRELESS_INTERFACE_VIDEO_AUTO_HOPPING_Handler,
    WIRELESS_INTERFACE_VIDEO_BAND_WIDTH_Handler,
    WIRELESS_INTERFACE_RESET_BB_Handler,
    WIRELESS_INTERFACE_OPERATE_REG_Handler,
    WIRELESS_INTERFACE_READ_RF9363_Handler,
    WIRELESS_INTERFACE_WRITE_RF9363_Handler,    
    NULL,
    NULL,
    NULL,
    NULL,
    PAD_FREQUENCY_BAND_WIDTH_SELECT_Handler,
    PAD_FREQUENCY_BAND_OPERATION_MODE_Handler,
    PAD_FREQUENCY_BAND_SELECT_Handler,
    PAD_FREQUENCY_CHANNEL_OPERATION_MODE_Handler,
    PAD_FREQUENCY_CHANNEL_SELECT_Handler,
    PAD_MCS_OPERATION_MODE_Handler,
    PAD_MCS_MODULATION_MODE_Handler,
    PAD_ENCODER_DYNAMIC_BITRATE_MODE_Handler,
    PAD_ENCODER_DYNAMIC_BITRATE_SELECT_Handler,
    PAD_WIRELESS_OSD_DISPLAY_Handler
};


uint8_t WIRELESS_IsInDebugMode(void)
{
    g_pstWirelessInfoDisplay  = (STRU_WIRELESS_INFO_DISPLAY *)OSD_STATUS_SHM_ADDR;

    return g_pstWirelessInfoDisplay->in_debug;
}


/* get osd info from shared memory */
uint8_t WIRELESS_GetOSDInfo(void)
{
    g_pstWirelessInfoDisplay  = (STRU_WIRELESS_INFO_DISPLAY *)OSD_STATUS_SHM_ADDR;

    /* if cpu2 update info, and the info is valid */
    if ((0x0 == g_pstWirelessInfoDisplay->head)
      &&(0xFF == g_pstWirelessInfoDisplay->tail)
      &&(0x0 == g_pstWirelessInfoDisplay->in_debug))
    {
        memcpy((void *)&g_stWirelessInfoSend, (void *)g_pstWirelessInfoDisplay, sizeof(STRU_WIRELESS_INFO_DISPLAY));

        return 0;
    }

    return 1;
}

/* Send to PAD or PC */
uint8_t WIRELESS_SendOSDInfo(uint8_t usbPortId)
{
    uint8_t               *u8_sendBuff;
    uint32_t               u32_sendLength;

    if (WIRELESS_GetOSDInfo())
    {
        dlog_error("osd info not ready");

        return 1;
    }

    u8_sendBuff                     = (uint8_t *)&g_stWirelessInfoSend;
    u32_sendLength                  = (uint32_t)(sizeof(STRU_WIRELESS_INFO_DISPLAY));

    g_stWirelessInfoSend.messageId  = WIRELESS_INTERFACE_OSD_DISPLAY;
    g_stWirelessInfoSend.paramLen   = u32_sendLength;

    if (HAL_OK != HAL_USB_DeviceSendCtrl(u8_sendBuff, u32_sendLength, usbPortId))
    {
        dlog_error("send osd info error");

        return 1;
    }

    return 0;
}


void UpgradeFirmwareFromPCTool(void *upgradeData, uint8_t u8_usbPortId)
{
    uint8_t             u8_replyToHost;
    uint8_t             u8_finalPacket;
    uint16_t            u16_packetLen;
    static uint8_t     *u8_sdramAddr = HID_UPGRADE_BASE_ADDR;
    uint32_t            u32_ImageSize;
    uint8_t            *u8_imageRawData;
    uint8_t             u8_md5sum[16];
    static uint32_t     u32_RecCount = 0;       // image size count
    uint32_t            u32_RecCountImage;      // image raw data size, exclude header
    uint8_t             u8_i;
    uint8_t            *u8_temp;

    u8_replyToHost      = *(uint8_t *)upgradeData;
    u8_finalPacket      = *((uint8_t *)upgradeData + 1);
    u16_packetLen       = *((uint16_t *)((uint8_t *)upgradeData + 2));

    if (u8_replyToHost != WIRELESS_INTERFACE_UPGRADE)
    {
        dlog_error("it is not a upgrade packet");

        return;
    }

    /* print this message at the first time */
    if (HID_UPGRADE_BASE_ADDR == u8_sdramAddr)
    {
        dlog_info("receiving image");
    }

    /* copy to SDRAM */
    memcpy(u8_sdramAddr,
           ((uint8_t *)upgradeData + 4),
           u16_packetLen);

    u8_sdramAddr       += u16_packetLen;
    u32_RecCount       += u16_packetLen;

    /* app image all received in SDRAM, start to upgrade app from SDRAM */
    if (u8_finalPacket)
    {
        dlog_info("CRC checking");

        /* 1-1. pointer to the image raw data */
        u8_imageRawData     = (HID_UPGRADE_BASE_ADDR + 34);

        /* 1-2. image size exclude header */
        u32_RecCountImage   = (u32_RecCount - 34);

        /* 1-3. get md5 sum from image header */
        u8_temp             = (HID_UPGRADE_BASE_ADDR + 18);
        for(u8_i = 0; u8_i < 16; u8_i++)
        {
            u8_md5sum[u8_i] = *(u8_temp + u8_i);
        }

        /* 1-4. get image size from image header */
        u8_temp         = (HID_UPGRADE_BASE_ADDR + 14);
        u32_ImageSize   = (((uint32_t)(*u8_temp)) | \
                           (((uint32_t)(*(u8_temp + 1))) << 8) | \
                           (((uint32_t)(*(u8_temp + 2))) << 16) | \
                           (((uint32_t)(*(u8_temp + 3))) << 24));

        if (u32_ImageSize != u32_RecCount)
        {
            dlog_error("received length incorrect");
        }

        /* 1-5. check MD5 */
        MD5Check(u8_imageRawData,
                 u32_RecCountImage,
                 u8_md5sum);

        /* 2-1. burn to norflash */
        for(u8_i = 0; u8_i < (u32_RecCount/HID_UPGRADE_FLASH_SECTOR_SIZE); u8_i++)
        {
            NOR_FLASH_EraseSector(HID_UPGRADE_APP_ADDR_IN_NOR + HID_UPGRADE_FLASH_SECTOR_SIZE*u8_i);
            NOR_FLASH_WriteByteBuffer((HID_UPGRADE_APP_ADDR_IN_NOR + HID_UPGRADE_FLASH_SECTOR_SIZE*u8_i),
                                      (HID_UPGRADE_BASE_ADDR + HID_UPGRADE_FLASH_SECTOR_SIZE*u8_i),
                                      HID_UPGRADE_FLASH_SECTOR_SIZE);
        }

        if(0 != (u32_RecCount%HID_UPGRADE_FLASH_SECTOR_SIZE))
        {
            NOR_FLASH_EraseSector(HID_UPGRADE_APP_ADDR_IN_NOR + HID_UPGRADE_FLASH_SECTOR_SIZE*u8_i);
            NOR_FLASH_WriteByteBuffer((HID_UPGRADE_APP_ADDR_IN_NOR + HID_UPGRADE_FLASH_SECTOR_SIZE*u8_i),
                                      (HID_UPGRADE_BASE_ADDR + HID_UPGRADE_FLASH_SECTOR_SIZE*u8_i),
                                      HID_UPGRADE_FLASH_SECTOR_SIZE);
        }

        /* re-check image MD5 in Norflash */
        /* 3-1. pointer to the image raw data */
        u8_temp             = (HID_UPGRADE_FLASH_BASE_ADDR + HID_UPGRADE_APP_ADDR_IN_NOR);
        u8_imageRawData     = (u8_temp + 34);

        /* 3-2. get md5 sum from image header */
        u8_temp             = ((HID_UPGRADE_FLASH_BASE_ADDR + HID_UPGRADE_APP_ADDR_IN_NOR) + 18);
        for(u8_i = 0; u8_i < 16; u8_i++)
        {
            u8_md5sum[u8_i] = *((uint8_t*)(u8_temp + u8_i));
        }

        /* 3-3. get image size from image header */
        u8_temp         = ((HID_UPGRADE_FLASH_BASE_ADDR + HID_UPGRADE_APP_ADDR_IN_NOR) + 14);
        u32_ImageSize   = (((uint32_t)(*u8_temp)) | \
                           (((uint32_t)(*(u8_temp + 1))) << 8) | \
                           (((uint32_t)(*(u8_temp + 2))) << 16) | \
                           (((uint32_t)(*(u8_temp + 3))) << 24));

        u32_RecCountImage   = (u32_ImageSize - 34);

        /* 3-4. check MD5 again */
        MD5Check(u8_imageRawData,
                 u32_RecCountImage,
                 u8_md5sum);

        dlog_info("upgrade app success");
    }

    while(HAL_OK != (HAL_USB_DeviceSendCtrl((uint8_t *)upgradeData, 4, u8_usbPortId)))
    {
        dlog_error("upgrade reply to host fail");
    }

}
 


uint8_t WIRELESS_INTERFACE_UPGRADE_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE   st_replyMessage;

    dlog_info("enter upgrade mode");

    HAL_USB_RegisterUserProcess(UpgradeFirmwareFromPCTool, NULL);

    st_replyMessage.messageId   = WIRELESS_INTERFACE_UPGRADE;
    st_replyMessage.paramLen    = 0;

    HAL_USB_DeviceSendCtrl((uint8_t *)&st_replyMessage, sizeof(STRU_WIRELESS_PARAM_CONFIG_MESSAGE), id);

    return 0;
}


uint8_t WIRELESS_INTERFACE_ENTER_TEST_MODE_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_ENTER_TEST_MODE_Handler\n"); 

    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (WIRELESS_IsInDebugMode() == 1)
    {
        HAL_BB_WriteByte(PAGE2, 0x02, 0x06);
    }
    else
    {
        HAL_BB_SetItOnlyFreqProxy(1);
    }

    return 0;
}


uint8_t WIRELESS_INTERFACE_SYNCHRONIZE_FREQ_CHANNEL_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SYNCHRONIZE_FREQ_CHANNEL_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_AUTO_MODE_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_AUTO_MODE_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SELF_ADAPTION_MODE_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SELF_ADAPTION_MODE_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SWITCH_TX_MODE_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_TX_MODE_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SWITCH_RX_MODE_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_RX_MODE_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SWITCH_USB1_MODE_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_USB1_MODE_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SWITCH_USB2_MODE_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_USB2_MODE_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_ALL_RESET_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_ALL_RESET_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_RX_RESET_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_RX_RESET_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_TX_RESET_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_TX_RESET_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_MIMO_1T2R_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_MIMO_1T2R_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_WRITE_BB_REG_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (WIRELESS_IsInDebugMode() == 1)
    {
        if (HAL_BB_CurPageWriteByte(recvMessage->paramData[0], recvMessage->paramData[1]))
        {
            dlog_error("write fail!\n");

            return 1;
        }
    }
    else
    {
        // dlog_info("inDebugFlag1 = %x\n",inDebugFlag);
        recvMessage->messageId = 0x0e;
        recvMessage->paramLen = 0;

        Wireless_InsertMsgIntoReplyBuff(recvMessage, id);
    }

    return 0;
}


uint8_t WIRELESS_INTERFACE_READ_BB_REG_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (WIRELESS_IsInDebugMode() == 1)
    {
        recvMessage->messageId = 0x0f;
        recvMessage->paramLen = 2;
        recvMessage->paramData[0] = recvMessage->paramData[0];
        HAL_BB_CurPageReadByte(recvMessage->paramData[0], &recvMessage->paramData[1]);

        Wireless_InsertMsgIntoReplyBuff(recvMessage, id);
    }
    else
    {   
        recvMessage->messageId = 0x0f;
        recvMessage->paramLen = 0;

        Wireless_InsertMsgIntoReplyBuff(recvMessage, id);
    }

    return 0;
}


uint8_t WIRELESS_INTERFACE_MIMO_2T2R_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_MIMO_1T2R_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_OSD_DISPLAY_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;

    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (id > HAL_USB_PORT_NUM)
    {
        dlog_error("error usb port id");

        return 1;
    }

    g_pstWirelessInfoDisplay  = (STRU_WIRELESS_INFO_DISPLAY *)OSD_STATUS_SHM_ADDR;

    if (recvMessage->paramData[0] == 0)
    {
        dlog_info("close osd: %d", id);

        g_u8OSDEnable[id] = 0;

        if ((g_u8OSDEnable[0] == 0)&&
            (g_u8OSDEnable[1] == 0))
        {
            g_pstWirelessInfoDisplay->osd_enable = 0;
        }
    }
    else
    {
        dlog_info("open osd: %d", id);

        g_u8OSDEnable[id] = 1;
        g_pstWirelessInfoDisplay->osd_enable = 1;
    }

    return 0;
}


uint8_t WIRELESS_INTERFACE_GET_ID_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_GET_ID_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_GET_GROUND_TX_PWR_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_GET_GROUND_TX_PWR_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_GET_SKY_TX_PWR_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_GET_SKY_TX_PWR_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_GET_RC_FREQ_CHANNEL_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_GET_RC_FREQ_CHANNEL_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_GET_VIDEO_FREQ_CHANNEL_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_GET_VIDEO_FREQ_CHANNEL_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_GET_SOFTWARE_VERSION_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_GET_SOFTWARE_VERSION_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_GET_DEV_INFO_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE        *stDeviceInfo;

    stDeviceInfo            = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)DEVICE_INFO_SHM_ADDR;

    stDeviceInfo->messageId = WIRELESS_INTERFACE_GET_DEV_INFO;
    stDeviceInfo->paramLen  = 28;

    dlog_info("WIRELESS_INTERFACE_GET_DEV_INFO_Handler\n");

    Wireless_InsertMsgIntoReplyBuff(stDeviceInfo, id);

    return 0;
}


uint8_t WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_MASK_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_MASK_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SET_VIDEO_FREQ_CHANNEL_MASK_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SET_VIDEO_FREQ_CHANNEL_MASK_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SELECT_VIDEO_FREQ_CHANNEL_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;
    uint8_t u8_index;
    uint32_t u32_freValue = 0;

    if (WIRELESS_IsInDebugMode() == 1)
    {
        for (u8_index = 0; u8_index < recvMessage->paramData[2]; ++u8_index)
        {
            HAL_BB_WriteByte(recvMessage->paramData[0],recvMessage->paramData[1],recvMessage->paramData[u8_index+3]);
            recvMessage->paramData[1] += 1;
        }
    }
    else
    {   
        for (u8_index = 0; u8_index < recvMessage->paramData[2]; ++u8_index)
        {
            u32_freValue = (u32_freValue << 8) + recvMessage->paramData[u8_index+3];
        }
        HAL_BB_SetItFreqProxy(u32_freValue);
    }

    return 0;
}


uint8_t WIRELESS_INTERFACE_SWITCH_QAM_MODE_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_QAM_MODE_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;
    uint8_t u8_index;
    uint32_t u32_freValue = 0;

    if (WIRELESS_IsInDebugMode() == 1)
    {
        for (u8_index = 0; u8_index < recvMessage->paramData[2]; ++u8_index)
        {
            HAL_BB_WriteByte(recvMessage->paramData[0],recvMessage->paramData[1],recvMessage->paramData[u8_index+3]);
            recvMessage->paramData[1] += 1;
        }
    }
    else
    {   
        for (u8_index = 0; u8_index < recvMessage->paramData[2]; ++u8_index)
        {
            u32_freValue = (u32_freValue << 8) + recvMessage->paramData[u8_index+3];
        }
        HAL_BB_SetRcFreqProxy(u32_freValue);
    }

    return 0;
}


uint8_t WIRELESS_INTERFACE_SET_TX_PWR_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SET_TX_PWR_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SET_VIDEO_TX_ID_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SET_VIDEO_TX_ID_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SET_CTRL_ID_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SET_CTRL_ID_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_SWITCH_WORKING_FREQ_BAND_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    ENUM_RF_BAND                         enRfBand;

    recvMessage                 = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (recvMessage->paramData[0] == 0)
    {
        dlog_info("2.4 G\n");
        enRfBand = RF_2G;
    }
    else
    {
        dlog_info("5.8 G\n");
        enRfBand = RF_5G;
    }

    HAL_BB_SetFreqBandProxy(enRfBand);

    return 0;
}


uint8_t WIRELESS_INTERFACE_RC_SCAN_ALL_BAND_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_RC_SCAN_ALL_BAND_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_RC_SCAN_WORKING_BAND_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_RC_SCAN_WORKING_BAND_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_VIDEO_SCAN_ALL_BAND_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_VIDEO_SCAN_ALL_BAND_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_VIDEO_SCAN_WORKING_BAND_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_VIDEO_SCAN_WORKING_BAND_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_RECOVER_TO_FACTORY_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_RECOVER_TO_FACTORY_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_RC_HOPPING_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    ENUM_RUN_MODE                        e_mode;

    recvMessage     = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (recvMessage->paramData[0] == 1)
    {
        e_mode      = AUTO;

        dlog_info("RC HOPPING MODE\n");
    }
    else
    {
        e_mode      = MANUAL;

        dlog_info("RC SELECT MODE\n");
    }

    //HAL_BB_SetItChannelSelectionModeProxy(e_mode);
    HAL_BB_SetRcChannelSelectionModeProxy(e_mode);
    return 0;
}


uint8_t WIRELESS_INTERFACE_SAVE_CONFIGURE_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SAVE_CONFIGURE_Handler\n");

    return 0;
}

uint8_t WIRELESS_INTERFACE_READ_MCU_ID_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_READ_MCU_ID_Handler\n");

    return 0;
}

uint8_t WIRELESS_INTERFACE_SWITCH_DEBUG_MODE_Handler(void *param, uint8_t id)
{
    uint8_t inDebugFlag = 0;

    STRU_WIRELESS_CONFIG_CHANGE           stWirelessConfigChange;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE    *sendMessage,*recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (recvMessage->paramData[0] == 0 && recvMessage->paramData[1] == 0)
    {
        /*enter debug mode */
        if (!eventFlag)
        {
           HAL_BB_SetBoardDebugModeProxy(0);
           eventFlag = 1;
        }

        inDebugFlag = 1;
    }
    else if (recvMessage->paramData[0] != 0 && recvMessage->paramData[1] == 0)
    {
        /*exit debug mode */
        if (eventFlag)
        {
            HAL_BB_SPI_DisableEnable(0); //
            HAL_BB_SetBoardDebugModeProxy(1);  
            eventFlag = 0;
        }

        inDebugFlag = 0;
    }

    /*send to PC*/
    recvMessage->paramData[1] = inDebugFlag;

    Wireless_InsertMsgIntoReplyBuff(recvMessage, id);

    return 0;
}

uint8_t WIRELESS_INTERFACE_WRITE_RF9363_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    uint16_t u16_addr = ((recvMessage->paramData[0]) << 8) | recvMessage->paramData[1];

    //dlog_info("ID = 0x%x 0x%x 0x%x 0x%x 0x%x\n",recvMessage->messageId, recvMessage->paramData[0], 
    //                                            recvMessage->paramData[1], recvMessage->paramData[2], u16_addr);

    if (WIRELESS_IsInDebugMode() == 1)
    {
        if (HAL_RF8003S_WriteReg(u16_addr, recvMessage->paramData[2]))
        {
            dlog_error("write fail!\n");
            return 1;
        }
    }
}
uint8_t WIRELESS_INTERFACE_WRITE_RF_REG_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    dlog_info("ID = %x\n",recvMessage->messageId);

    if (WIRELESS_IsInDebugMode() == 1)
    {
        if (HAL_RF8003S_WriteReg(recvMessage->paramData[0], recvMessage->paramData[1]))
        {
            dlog_error("write fail!\n");

            return 1;
        }
    }
    else
    {
        // dlog_info("inDebugFlag1 = %x\n",inDebugFlag);
        recvMessage->messageId = 0x0e;
        recvMessage->paramLen = 0;

        Wireless_InsertMsgIntoReplyBuff(recvMessage, id);
    }

    return 0;
}


uint8_t WIRELESS_INTERFACE_OPEN_VIDEO_Handler(void *param, uint8_t id)
{
    HAL_USB_OpenVideo(id);

    return 0;
}


uint8_t WIRELESS_INTERFACE_CLOSE_VIDEO_Handler(void *param, uint8_t id)
{
    HAL_USB_CloseVideo(id);

    return 0;
}


uint8_t WIRELESS_INTERFACE_READ_RF9363_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;
    uint16_t addr = (recvMessage->paramData[0] << 8) | recvMessage->paramData[1];

    if (WIRELESS_IsInDebugMode() == 1)
    {

        recvMessage->messageId = 0x4A;
        recvMessage->paramLen  = 3;
        recvMessage->paramData[0] = recvMessage->paramData[0];
        recvMessage->paramData[1] = recvMessage->paramData[1];
        
        HAL_RF8003S_ReadByte(addr, &recvMessage->paramData[2]);
        Wireless_InsertMsgIntoReplyBuff(recvMessage, id);
    }
}


uint8_t WIRELESS_INTERFACE_READ_RF_REG_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;
    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (WIRELESS_IsInDebugMode() == 1)
    {
        recvMessage->messageId = 0x3A;
        recvMessage->paramLen = 2;
        recvMessage->paramData[0] = recvMessage->paramData[0];
        HAL_RF8003S_ReadByte(recvMessage->paramData[0],&recvMessage->paramData[1]);

        Wireless_InsertMsgIntoReplyBuff(recvMessage, id);
    }
    else
    {
        recvMessage->messageId = 0x0f;
        recvMessage->paramLen = 0;

        Wireless_InsertMsgIntoReplyBuff(recvMessage, id);
    }

    return 0;
}
uint8_t WIRELESS_INTERFACE_LOAD_SKY_REG_TABLE_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_LOAD_SKY_REG_TABLE_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_LOAD_GRD_REG_TABLE_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_LOAD_GRD_REG_TABLE_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_SWITCH_BB_POWER_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_BB_POWER_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_SKY_ONLY_RX_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SKY_ONLY_RX_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_SWITCH_RF_PWR_0_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_RF_PWR_0_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_SWITCH_RF_PWR_1_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_RF_PWR_1_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_EXT_ONEKEY_IT_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_EXT_ONEKEY_IT_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_SWITCH_IT_CHAN_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_IT_CHAN_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_SWITCH_RMT_CHAN_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SWITCH_RMT_CHAN_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_SET_PWR_CAL_0_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SET_PWR_CAL_0_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_SET_PWR_CAL_1_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_SET_PWR_CAL_1_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_RST_MCU_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_RST_MCU_Handler\n");

    return 0;
}
uint8_t WIRELESS_INTERFACE_RF_PWR_AUTO_Handler(void *param, uint8_t id)
{
    dlog_info("WIRELESS_INTERFACE_RF_PWR_AUTO_Handler\n");

    return 0;
}


uint8_t WIRELESS_INTERFACE_OPEN_ADAPTION_BIT_STREAM_Handler(void *param, uint8_t id)
{
    ENUM_RUN_MODE                        enRunMode;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;

    recvMessage             = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (recvMessage->paramData[0] == 1)
    {
        enRunMode           = AUTO;

        dlog_info("auto\n");
    }
    else
    {
        enRunMode           = MANUAL;

        dlog_info("manual\n");
    }

    //HAL_BB_SetEncoderBrcModeProxy(enRunMode);
    HAL_BB_SetMcsModeProxy(enRunMode);

    return 0;
}

uint8_t WIRELESS_INTERFACE_SWITCH_CH1_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    recvMessage     = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;
    uint8_t u8_data = 0;

    dlog_info("WIRELESS_INTERFACE_SWITCH_CH1_Handler");

    if (recvMessage->paramData[0] == 0)
    {
        dlog_info("ch1 off");
        u8_data = 0;
    }
    else
    {
        dlog_info("ch1 on");
        u8_data = 1;
    }

    HAL_BB_SwitchOnOffChProxy(0, u8_data);

    return 0;
}

uint8_t WIRELESS_INTERFACE_SWITCH_CH2_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    recvMessage     = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;
    uint8_t u8_data = 0;

    dlog_info("WIRELESS_INTERFACE_SWITCH_CH2_Handler");

    if (recvMessage->paramData[0] == 0)
    {
        dlog_info("ch1 off");
        u8_data = 0;
    }
    else
    {
        dlog_info("ch1 on");
        u8_data = 1;
    }

    HAL_BB_SwitchOnOffChProxy(1, u8_data);

    return 0;
}

uint8_t WIRELESS_INTERFACE_SET_CH1_BIT_RATE_Handler(void *param, uint8_t id)
{
    dlog_info("set ch1 bit rate");

    HAL_BB_SetEncoderBitrateProxy(0, (uint8_t)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}

uint8_t WIRELESS_INTERFACE_SET_CH2_BIT_RATE_Handler(void *param, uint8_t id)
{
    dlog_info("set ch2 bit rate");

    HAL_BB_SetEncoderBitrateProxy(1, (uint8_t)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}

uint8_t WIRELESS_INTERFACE_VIDEO_QAM_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    ENUM_BB_QAM                          enBBQAM;

    recvMessage         = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    enBBQAM             = (ENUM_BB_QAM)(recvMessage->paramData[0] & 0x3);

    dlog_info("QAM : %d", recvMessage->paramData[0]);

    HAL_BB_SetFreqBandQamSelectionProxy(enBBQAM);

    return 0;
}

uint8_t WIRELESS_INTERFACE_VIDEO_CODE_RATE_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    ENUM_BB_LDPC                         enldpc;

    recvMessage         = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    enldpc              = (ENUM_BB_LDPC)recvMessage->paramData[0];

    dlog_info("ldpc: %d", recvMessage->paramData[0]);

    HAL_BB_SetItLdpcProxy(enldpc);

    return 0;
}

uint8_t WIRELESS_INTERFACE_RC_QAM_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    ENUM_BB_QAM                          enBBQAM;

    recvMessage         = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    enBBQAM             = (ENUM_BB_QAM)(recvMessage->paramData[0] & 0x1);

    dlog_info("RC QAM : %d", recvMessage->paramData[0]);

    HAL_BB_SetRcQamSelectionProxy(enBBQAM);

    return 0;
}

uint8_t WIRELESS_INTERFACE_RC_CODE_RATE_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    ENUM_BB_LDPC                         e_ldpc;

    recvMessage         = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    dlog_info("RC CODE RATE: %d", recvMessage->paramData[0]);

    switch (recvMessage->paramData[0])
    {
        case 0:
            e_ldpc      = LDPC_1_2;
            break;

        case 1:
            e_ldpc      = LDPC_2_3;        
            break;

        default:
            e_ldpc      = LDPC_1_2;
            break;
    }

    HAL_BB_SetRcLdpcProxy(e_ldpc);

    return 0;
}

uint8_t WIRELESS_INTERFACE_VIDEO_AUTO_HOPPING_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    ENUM_RUN_MODE                        e_mode;

    recvMessage     = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (recvMessage->paramData[0] == 1)
    {
        e_mode      = AUTO;

        dlog_info("VIDEO HOPPING MODE\n");
    }
    else
    {
        e_mode      = MANUAL;

        dlog_info("VIDEO SELECT MODE\n");
    }

    HAL_BB_SetItChannelSelectionModeProxy(e_mode);

    return 0;
}

uint8_t WIRELESS_INTERFACE_VIDEO_BAND_WIDTH_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    ENUM_CH_BW                           enBandWidth;

    recvMessage                 = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    if (recvMessage->paramData[0] == 2)
    {
        dlog_info("10M Bandwidth\n");
        enBandWidth             = BW_10M;
    }
    else
    {
        dlog_info("20M Bandwidth\n");
        enBandWidth             = BW_20M;
    }

	HAL_BB_SetFreqBandwidthSelectionProxy(enBandWidth);

    return 0;
}


uint8_t WIRELESS_INTERFACE_RESET_BB_Handler(void *param, uint8_t id)
{
    dlog_info("reset bb");
    HAL_BB_SoftResetProxy();
    return 0;
}


uint8_t WIRELESS_INTERFACE_OPERATE_REG_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE  *recvMessage;
    uint32_t                            *regAddr;
    uint32_t                             regValue;
    uint32_t                             temp = 0;
    uint8_t                              addrOffset;

    recvMessage                 = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    addrOffset                  = recvMessage->paramData[1];

    regAddr                     = (uint32_t *)(0xA0010000 + (addrOffset - (addrOffset & 0x3)));

    addrOffset                 &= 0x3;
    /* little-big endian change */
    addrOffset                  = (3 - addrOffset);

    regValue                    = *(uint32_t *)regAddr;

    /* read register */
    if (recvMessage->paramData[0] == 0)
    {
        temp                    = regValue;
        temp                  >>= (addrOffset << 3);

        recvMessage->paramData[2] = (uint8_t)temp;
    }
    /* write register */
    else if (recvMessage->paramData[0] == 1)
    {
        temp                   = recvMessage->paramData[2];
        temp                  &= 0xFF;
        temp                 <<= (addrOffset << 3);

        regValue              &= ~(0xFF << (addrOffset << 3));
        regValue              |= temp;

        *regAddr               = regValue;
    }
    else
    {
        return 1;
    }

    Wireless_InsertMsgIntoReplyBuff(recvMessage, id);

    return 0;
}


uint8_t PAD_FREQUENCY_BAND_WIDTH_SELECT_Handler(void *param, uint8_t id)
{
	HAL_BB_SetFreqBandwidthSelectionProxy( (ENUM_CH_BW)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_FREQUENCY_BAND_OPERATION_MODE_Handler(void *param, uint8_t id)
{
	HAL_BB_SetFreqBandSelectionModeProxy( (ENUM_RUN_MODE)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_FREQUENCY_BAND_SELECT_Handler(void *param, uint8_t id)
{
	HAL_BB_SetFreqBandProxy( (ENUM_RF_BAND)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_FREQUENCY_CHANNEL_OPERATION_MODE_Handler(void *param, uint8_t id)
{
	HAL_BB_SetItChannelSelectionModeProxy( (ENUM_RUN_MODE)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_FREQUENCY_CHANNEL_SELECT_Handler(void *param, uint8_t id)
{	
	HAL_BB_SetItChannelProxy( (uint8_t)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_MCS_OPERATION_MODE_Handler(void *param, uint8_t id)
{
	HAL_BB_SetMcsModeProxy( (ENUM_RUN_MODE)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_MCS_MODULATION_MODE_Handler(void *param, uint8_t id)
{
	HAL_BB_SetItQamProxy( (ENUM_BB_QAM)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_ENCODER_DYNAMIC_BITRATE_MODE_Handler(void *param, uint8_t id)
{
    HAL_BB_SetEncoderBrcModeProxy( (ENUM_RUN_MODE)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_ENCODER_DYNAMIC_BITRATE_SELECT_Handler(void *param, uint8_t id)
{
    HAL_BB_SetEncoderBitrateProxy(0, (uint8_t)(((STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param)->paramData[0]));

    return 0;
}


uint8_t PAD_WIRELESS_OSD_DISPLAY_Handler(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *recvMessage;

    recvMessage = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    g_pstWirelessInfoDisplay  = (STRU_WIRELESS_INFO_DISPLAY *)OSD_STATUS_SHM_ADDR;

    if (recvMessage->paramData[0] == 0)
    {
        dlog_info("close osd");

        g_pstWirelessInfoDisplay->osd_enable    = 0;
    }
    else
    {
        dlog_info("open osd");

        g_pstWirelessInfoDisplay->osd_enable    = 1;
    }

    return 0;
}

void WIRELESS_ParseParamConfig(void *param, uint8_t id)
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *pstWirelessParamConfig;

	pstWirelessParamConfig          = (STRU_WIRELESS_PARAM_CONFIG_MESSAGE *)param;

    // insert message to the buffer tail
    memcpy((void *)&g_stWirelessParamConfig.stMsgPool[g_stWirelessParamConfig.u8_buffTail],
           (void *)pstWirelessParamConfig,
           sizeof(STRU_WIRELESS_PARAM_CONFIG_MESSAGE));

    g_stWirelessParamConfig.u8_usbPortId[g_stWirelessParamConfig.u8_buffTail] = id;
    g_stWirelessParamConfig.u8_buffTail++;
    g_stWirelessParamConfig.u8_buffTail &= (WIRELESS_INTERFACE_MAX_MESSAGE_NUM - 1);

    if (g_stWirelessParamConfig.u8_buffTail == g_stWirelessParamConfig.u8_buffHead)
    {
        dlog_error("wireless buff is full");
    }

    return;
}


void Wireless_MessageProcess(void)
{
    uint8_t                                 messageId;
    uint8_t                                *u8_sendBuff;
    uint8_t                                 debugMode = 0;
    uint32_t                                u32_sendLength;
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE     *pstWirelessParamConfig;
    uint8_t                                 u8_usbPortId;

    g_pstWirelessInfoDisplay  = (STRU_WIRELESS_INFO_DISPLAY *)OSD_STATUS_SHM_ADDR;

    if ((HAL_USB_DeviceGetConnState(0) == 0)&&
         (HAL_USB_DeviceGetConnState(1) == 0))
    {
        return;
    }

    if (g_stWirelessReply.u8_buffTail != g_stWirelessReply.u8_buffHead)
    {
        pstWirelessParamConfig = &g_stWirelessReply.stMsgPool[g_stWirelessReply.u8_buffHead];

        messageId              = pstWirelessParamConfig->messageId;

        u8_sendBuff         = (uint8_t *)pstWirelessParamConfig;
        u32_sendLength      = (uint32_t)sizeof(STRU_WIRELESS_PARAM_CONFIG_MESSAGE);

        if (HAL_OK != HAL_USB_DeviceSendCtrl(u8_sendBuff, u32_sendLength, g_stWirelessReply.u8_usbPortId[g_stWirelessReply.u8_buffHead]))
        {
            dlog_error("send wireless info fail");
        }
        else
        {
            g_stWirelessReply.u8_buffHead++;
            g_stWirelessReply.u8_buffHead &= (WIRELESS_INTERFACE_MAX_MESSAGE_NUM - 1);
        }
    }
    else if (g_pstWirelessInfoDisplay->osd_enable)
    {
        g_u8OSDToggle  ^= 1;

        if (g_u8OSDToggle == 1)
        {
            if (g_u8OSDEnable[0])
            {
                WIRELESS_SendOSDInfo(0);
            }
        }
        else
        {
            if (g_u8OSDEnable[1])
            {
                WIRELESS_SendOSDInfo(1);
            }
        }
    }

    if (g_stWirelessParamConfig.u8_buffTail != g_stWirelessParamConfig.u8_buffHead)
    {
        // get the head node from the buffer
        pstWirelessParamConfig = &g_stWirelessParamConfig.stMsgPool[g_stWirelessParamConfig.u8_buffHead];

        messageId = pstWirelessParamConfig->messageId;

        if (messageId < MAX_PID_NUM)
        {
            if (g_stWirelessMsgHandler[messageId])
            {
                u8_usbPortId = g_stWirelessParamConfig.u8_usbPortId[g_stWirelessParamConfig.u8_buffHead];
            
                (g_stWirelessMsgHandler[messageId])(pstWirelessParamConfig, u8_usbPortId);
            }
            else
            {
                dlog_error("no this message handler,%d", messageId);
            }
        }
    
        g_stWirelessParamConfig.u8_buffHead++;
        g_stWirelessParamConfig.u8_buffHead &= (WIRELESS_INTERFACE_MAX_MESSAGE_NUM - 1);
    }
    
    if (debugMode != (g_pstWirelessInfoDisplay->in_debug))
    {
        debugMode = g_pstWirelessInfoDisplay->in_debug;
        if (1 == debugMode)
        {
            HAL_BB_SPI_DisableEnable(debugMode);
        }
    }

}

static void Wireless_Task(void const *argument)
{
    dlog_info("wireless task entry");

    while (1)
    {
        Wireless_MessageProcess();

        HAL_Delay(5);
    }
}


void Wireless_InitBuffer(void)
{
    memset((void *)&g_stWirelessParamConfig, 0, sizeof(STRU_WIRELESS_MESSAGE_BUFF));
    memset((void *)&g_stWirelessReply, 0, sizeof(STRU_WIRELESS_MESSAGE_BUFF));

    g_pstWirelessInfoDisplay  = (STRU_WIRELESS_INFO_DISPLAY *)OSD_STATUS_SHM_ADDR;

    g_pstWirelessInfoDisplay->osd_enable = 0;
}

__attribute__((weak)) void ar_osWirelessTaskInit(void TaskHandler(void const *argument))
{
}

void Wireless_TaskInit(uint8_t u8_useRTOS)
{
    Wireless_InitBuffer();

    memset((void *)OSD_STATUS_SHM_ADDR, 0, 512);

    HAL_USB_RegisterUserProcess(WIRELESS_ParseParamConfig, Wireless_InitBuffer);

    if (u8_useRTOS)
    {
        ar_osWirelessTaskInit(Wireless_Task);
    }
}


static void Wireless_InsertMsgIntoReplyBuff(STRU_WIRELESS_PARAM_CONFIG_MESSAGE *pstMessage, uint8_t u8_usbPortId)
{
    // insert message to the buffer tail
    memcpy((void *)&g_stWirelessReply.stMsgPool[g_stWirelessReply.u8_buffTail],
           (void *)pstMessage,
           sizeof(STRU_WIRELESS_PARAM_CONFIG_MESSAGE));

    g_stWirelessReply.u8_usbPortId[g_stWirelessReply.u8_buffTail] = u8_usbPortId;

    g_stWirelessReply.u8_buffTail++;
    g_stWirelessReply.u8_buffTail &= (WIRELESS_INTERFACE_MAX_MESSAGE_NUM - 1);

    if (g_stWirelessReply.u8_buffTail == g_stWirelessReply.u8_buffHead)
    {
        dlog_error("reply buff is full");
        g_stWirelessReply.u8_buffHead = 0;
        g_stWirelessReply.u8_buffTail = 0;
    }

    return;
}




