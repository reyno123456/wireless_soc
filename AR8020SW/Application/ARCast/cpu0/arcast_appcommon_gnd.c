#include <stdint.h>
#include <string.h>

#include "debuglog.h"
#include "sys_event.h"
#include "memory_config.h"

#include "hal_ret_type.h"
#include "hal_encodemp3.h"
#include "hal_softi2s.h"
#include "hal_sram.h"
#include "hal_usb_device.h"
#include "hal_bb.h"
#include "hal.h"
#include "arcast_appcommon.h"

#define ARCAST_DEBUGE

#ifdef ARCAST_DEBUGE
#define DLOG_INFO(...) dlog_info(__VA_ARGS__)
#else
#define DLOG_INFO(...)
#endif

extern unsigned int s_st_ARCastSupportedOutputFormat[9][4];
static STRU_ARCAST_AVSTAUTS st_ARCastStatus;

static uint8_t u8_commandArray[32];

static void AR8020_SendFormatCommand(void);


static void RecvFormatAck(void *buff, uint32_t data_len, uint8_t  port_id)
{
    ATM_StatusCallBack(buff, data_len);
    
    return;
}

static HAL_RET_T ARCastGnd_sendCommand(uint8_t u8_command, uint8_t *pu8_Databuff, uint8_t u8_len)
{
    uint8_t i=0;
    memset(u8_commandArray, 0, sizeof(u8_commandArray));
    
    u8_commandArray[0] = 0x41;
    u8_commandArray[1] = 0x82;
    u8_commandArray[2] = u8_command;
    u8_commandArray[3] = u8_len+1;
    memcpy(&u8_commandArray[4], pu8_Databuff, u8_len);
    
    for (i = 0; i < 4 + u8_len; i++)
    {
        u8_commandArray[4 + u8_len] += u8_commandArray[i];
    }
    printf("send command: ");
    for (i = 0; i < 5 + u8_len; i++)
    {
        printf("%x ",u8_commandArray[i]);
    }
    printf("end\r\n");
   // return HAL_OK;
    return HAL_USB_CustomerSendData(u8_commandArray, 4 + u8_len, 0);

}

static int8_t CheckVideoFormatSupportOrNot(uint16_t u16_width, uint16_t u16_hight, uint8_t u8_framerate)
{
    uint8_t i = 0;
    uint8_t array_size = sizeof(s_st_ARCastSupportedOutputFormat)/sizeof(s_st_ARCastSupportedOutputFormat[0]);

    for (i = 0; i < array_size; i++)
    {
        if ((u16_width == s_st_ARCastSupportedOutputFormat[i][0]) &&
            (u16_hight == s_st_ARCastSupportedOutputFormat[i][1]) &&
            (u8_framerate == s_st_ARCastSupportedOutputFormat[i][2]))
        {
            return i;
        }
    }
    
    return -1;
    

}

static void rcvFormatHandler_ground(void *p)
{
    uint32_t u32_rcvLen = 0;
    uint8_t  u8_recData[64];
    HAL_BB_UartComReceiveMsg(BB_UART_COM_SESSION_3, u8_recData, 64, &u32_rcvLen);
    
    if (1 == u32_rcvLen)
    {
        switch (u8_recData[0])
        {
            case ARCAST_COMMAND_GND_CAPABILITY:
                {
                    st_ARCastStatus.u8_ARStatus = 0;
                    st_ARCastStatus.u8_ATMStatus = 0;      
                    st_ARCastStatus.u8_ARStatus |= (1 << ARCAST_COMMAND_GND_CAPABILITY); 
                    st_ARCastStatus.u8_ARStatus |= (1 << ARCAST_COMMAND_SKY_REQUEST_CAPABILITY); 
                    dlog_info("rec sky ack CAPABILITY");
                    break;
                }
            case ARCAST_COMMAND_SKY_REQUEST_CAPABILITY:
                {
                    st_ARCastStatus.u8_ARStatus = 0;
                    st_ARCastStatus.u8_ATMStatus = 0;
                    st_ARCastStatus.u8_ARStatus |= (1 << ARCAST_COMMAND_GND_CAPABILITY);  
                    st_ARCastStatus.u8_ARStatus |= (1 << ARCAST_COMMAND_SKY_REQUEST_CAPABILITY);      
                    HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_3, (uint8_t *)(&st_ARCastStatus.st_avCapability), sizeof(STRU_ARCAST_AVCAPABILITY));   
                    break;
                }
            case ARCAST_COMMAND_SKY_AVDATA:
                {
                    st_ARCastStatus.u8_ARStatus |= (1 << ARCAST_COMMAND_SKY_AVDATA);
                    st_ARCastStatus.u8_ARStatus &= (~(1 << ARCAST_COMMAND_SKY_FORMAT));
                    dlog_info("rec SKY_AVDATA");
                    break;
                }

        }
        
    }
    else if (2 == u32_rcvLen)
    {
        st_ARCastStatus.u8_AVFormat[0] = u8_recData[0];
        st_ARCastStatus.u8_AVFormat[1] = u8_recData[1];
        st_ARCastStatus.u8_ARStatus |= (1 << ARCAST_COMMAND_SKY_FORMAT);
        uint8_t ack = ARCAST_COMMAND_SKY_FORMAT;
        HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_3, &ack, 1);
        dlog_info("rec sky ack FORMAT");
    }
}

void Command_BBSendCommand(void const *argument)
{
    while(1)
    {
        HAL_Delay(2000);
        if (0 == (st_ARCastStatus.u8_ARStatus & (1 << ARCAST_COMMAND_GND_CAPABILITY)))
        {
            dlog_info("ARCAST_COMMAND_GND_CAPABILITY");  
            HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_3, (uint8_t *)(&st_ARCastStatus.st_avCapability), sizeof(STRU_ARCAST_AVCAPABILITY));
        }      
        else if ((st_ARCastStatus.u8_ARStatus & (1 << ARCAST_COMMAND_SKY_FORMAT)) || 
                 (0 == (st_ARCastStatus.u8_ARStatus & (1 << ARCAST_COMMAND_SKY_AVDATA))))
        {
            if (0 == (st_ARCastStatus.u8_ATMStatus & (1 << ARCAST_COMMAND_FORMAT)))
            {
                dlog_info("AR8020_SendFormatCommand");
                
                AR8020_SendFormatCommand();
            }
            else  if (0 == (st_ARCastStatus.u8_ARStatus & (1 << ARCAST_COMMAND_SKY_AVDATA)))
            {
                dlog_info("ARCAST_COMMAND_FORMAT");         
                uint8_t ack = ARCAST_COMMAND_SKY_AVDATA;
                HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_3, &ack, 1);
            }
            else
            {
                dlog_error("FORMAT"); 
            }
        }
    }
}

void Common_AVFORMATSysEventGroundInit(void)
{
    uint8_t u8_command = 0x03;

    HAL_BB_UartComRemoteSessionInit();      

    HAL_BB_UartComRegisterSession(BB_UART_COM_SESSION_3,
                                  BB_UART_SESSION_PRIORITY_HIGH,
                                  BB_UART_SESSION_DATA_NORMAL,
                                  rcvFormatHandler_ground);

    HAL_USB_RegisterCustomerRecvData(RecvFormatAck); 

    while (st_ARCastStatus.u8_ATMStatus & ARCAST_COMMAND_REPLY_EDID)
    {
        ARCastGnd_sendCommand(ARCAST_COMMAND_REQUEST_EDID, &u8_command, 1);
        HAL_Delay(1000);
    }

/*    st_ARCastStatus.st_avCapability.u8_VideoVidCount = 0x03;
    st_ARCastStatus.st_avCapability.u8_VideoVidList[0] = 0x01;
    st_ARCastStatus.st_avCapability.u8_VideoVidList[1] = 0x02;
    st_ARCastStatus.st_avCapability.u8_VideoVidList[2] = 0x04;

    st_ARCastStatus.st_avCapability.u8_AudioAidCount = 0x02;
    st_ARCastStatus.st_avCapability.u8_AudioAidList[0] = 0x01;
    st_ARCastStatus.st_avCapability.u8_AudioAidList[1] = 0x02;*/

    st_ARCastStatus.u8_ARStatus &= (~(1 << ARCAST_COMMAND_GND_CAPABILITY));
    dlog_info("send capability");
    HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_3, (uint8_t *)(&st_ARCastStatus.st_avCapability), sizeof(STRU_ARCAST_AVCAPABILITY));    
}

static void ATM_ACKStatus(uint8_t u8_status)
{
    st_ARCastStatus.u8_ATMStatus |= (1 << u8_status);
    dlog_info("ack command %x %x", u8_status, st_ARCastStatus.u8_ATMStatus);
}


static void AR8020_SendEDIDRequset(void)
{
    uint8_t u8_command = 0x03;
    
    while (HAL_OK != ARCastGnd_sendCommand(ARCAST_COMMAND_REQUEST_EDID, &u8_command, 1))
    {
        HAL_Delay(10);
    }
    
}


static void ATM_EDIDHandle(uint8_t *pu8_EDIDInfo, uint8_t u8_Len)
{
    uint8_t i = 0;
    uint8_t u8_videoCount = pu8_EDIDInfo[0];
    
    if ((u8_videoCount+1) == u8_Len)
    {
        dlog_warning("don't support audio");
    }
    else
    {
        st_ARCastStatus.st_avCapability.u8_AudioAidCount = pu8_EDIDInfo[u8_videoCount + 1];  
        memcpy(&st_ARCastStatus.st_avCapability.u8_AudioAidList, &pu8_EDIDInfo[u8_videoCount+1], st_ARCastStatus.st_avCapability.u8_AudioAidCount);       
    }

    st_ARCastStatus.st_avCapability.u8_VideoVidCount = u8_videoCount;    
    if (0 != u8_videoCount)
    {
        memcpy(&st_ARCastStatus.st_avCapability.u8_VideoVidList, &pu8_EDIDInfo[i+1], u8_videoCount);        
    }
    
    dlog_info("support video %d audio %d",st_ARCastStatus.st_avCapability.u8_VideoVidCount, st_ARCastStatus.st_avCapability.u8_AudioAidCount);
}

static void AR8020_SendFormatCommand(void)
{
    uint8_t u8_commandArray[2] = {st_ARCastStatus.u8_AVFormat[0], st_ARCastStatus.u8_AVFormat[1]};
    
    while (HAL_OK != ARCastGnd_sendCommand(ARCAST_COMMAND_FORMAT, u8_commandArray, 2))
    {
        HAL_Delay(1000);
    }

    
}
static void AR8020_ReplyACK(uint8_t u8_ackCommand)
{
    uint8_t u8_command = u8_ackCommand;

    while (HAL_OK != ARCastGnd_sendCommand(ARCAST_COMMAND_ACK, &u8_command, 1))
    {
        HAL_Delay(1000);
    }
}

static void ATM_Error(uint8_t u8_errorCommand)
{
    dlog_error("error command %x", u8_errorCommand);
    switch(u8_errorCommand)
    {
        case ARCAST_COMMAND_REQUEST_EDID:
            AR8020_SendEDIDRequset();
            break;

        case ARCAST_COMMAND_FORMAT:
            AR8020_SendFormatCommand();
            break;
    }
}

void ATM_StatusCallBack(uint8_t *st_status, uint32_t data_len)
{
    uint32_t i = 0;
    uint8_t  u8_checkSum = 0;
    for (i = 0; i < (data_len - 1); i++)
    {
        u8_checkSum += st_status[i];
    }

    if ((0x41 != st_status[0]) || (0x82 != st_status[1]) || (u8_checkSum != st_status[data_len]))
    {
        dlog_error("command error head %x %x CheckSum %x %x", st_status[0], st_status[1], u8_checkSum, st_status[data_len]);
        
        uint8_t u8_command = st_status[2];
        while (HAL_OK != ARCastGnd_sendCommand(ARCAST_COMMAND_ACK, &u8_command, 1))
        {
            HAL_Delay(1000);
        }
    }
    else
    {
        dlog_info("command %x", st_status[2]);
        switch (st_status[2])
        {
            case ARCAST_COMMAND_ACK:
                ATM_ACKStatus(st_status[4]);
                break;

            case ARCAST_COMMAND_REPLY_EDID:
                ATM_EDIDHandle(&st_status[4], st_status[3]-1);
                ATM_ACKStatus(st_status[2]);
                AR8020_ReplyACK(st_status[2]);
                break;
            
            case ARCAST_COMMAND_ERROR:
                ATM_Error(st_status[4]);
                break;

            default:
                break;
        }        
    }

}






