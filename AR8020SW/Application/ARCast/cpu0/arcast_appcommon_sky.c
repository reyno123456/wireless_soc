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
#include "it_typedef.h"
#include "it6602.h"
#include "eedid_create.h"

#define ARCAST_DEBUGE

#ifdef  ARCAST_DEBUGE
#define DLOG_INFO(...) dlog_info(__VA_ARGS__)
#else
#define DLOG_INFO(...)
#endif

extern unsigned char Default_Edid_Block[256];
extern unsigned int s_st_ARCastSupportedOutputFormat[9][4];
static STRU_ARCAST_AVSTAUTS st_ARCastStatus;

int8_t Common_AVFORMAT_VideoFormatCheck(void* p)
{
    for (uint8_t i = 0; i < ARRAY_COUNT_OF(s_st_ARCastSupportedOutputFormat); i++)
    {
        if (((STRU_SysEvent_H264InputFormatChangeParameter*)p)->vic == s_st_ARCastSupportedOutputFormat[i][3])
        {
            return i;
        }
        else if (s_st_ARCastSupportedOutputFormat[i][3] == 0xff)
        {
            if (((STRU_SysEvent_H264InputFormatChangeParameter*)p)->width == s_st_ARCastSupportedOutputFormat[i][0] &&
                ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->hight == s_st_ARCastSupportedOutputFormat[i][1] && 
                ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->framerate == s_st_ARCastSupportedOutputFormat[i][2])
            {
                return i;
            }
        }
    }
    return 0xff;
}
void Common_AVFORMAT_VideoSysEventCallBack(void* p)
{
    DLOG_INFO("video width=%d hight=%d framerate=%d vic=%d", ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->width, 
                                                             ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->hight, 
                                                             ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->framerate,
                                                             ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->vic);
    
    uint8_t u8_vid = Common_AVFORMAT_VideoFormatCheck(p);
    if (0xff == u8_vid )
    {
        dlog_error("rec u8_vid=%d",u8_vid); 
    }
    else if ((st_ARCastStatus.u8_AVFormat[0] != u8_vid) || (0 == (st_ARCastStatus.u8_ARStatus & (1 << ARCAST_COMMAND_SKY_REQUEST_CAPABILITY))))
    {
        st_ARCastStatus.u8_AVFormat[0] = u8_vid;
        st_ARCastStatus.u8_ARStatus &= (~(1 << ARCAST_COMMAND_SKY_FORMAT));
        st_ARCastStatus.u8_ARStatus &= (~(1 << ARCAST_SKY_ENCODER));
        STRU_SysEvent_H264InputFormatChangeParameter p;
        p.index = 1;
        p.width = 0;
        p.hight = 0;
        p.framerate = 0;
        SYS_EVENT_Notify(SYS_EVENT_ID_H264_INPUT_FORMAT_CHANGE, (void*)&p);
    }
}

void Common_AVFORMAT_AudioSysEventCallBack(void* p)
{
    HAL_MP3EncodePcmUnInit();
    STRU_MP3_ENCODE_CONFIGURE_WAVE st_audioConfig;
    if (HAL_SOFTI2S_ENCODE_IEC_48000 == ((STRU_SysEvent_AudioInputChangeParameter*)p)->u8_audioSampleRate)
    {                    
        st_audioConfig.e_samplerate = HAL_MP3_ENCODE_48000;
        st_ARCastStatus.u8_AVFormat[1] = 0x02;
        DLOG_INFO("Audio Sample Rate 48000");
    }
    else if (HAL_SOFTI2S_ENCODE_IEC_44100 == ((STRU_SysEvent_AudioInputChangeParameter*)p)->u8_audioSampleRate)
    {
        st_audioConfig.e_samplerate = HAL_MP3_ENCODE_44100; 
        st_ARCastStatus.u8_AVFormat[1] = 0x01;
        DLOG_INFO("Audio Sample Rate 44100");                 
    }
    
    st_audioConfig.e_modes = HAL_MP3_ENCODE_STEREO;
    st_audioConfig.u32_rawDataAddr = AUDIO_DATA_START;
    st_audioConfig.u32_rawDataLenght = AUDIO_DATA_BUFF_SIZE;
    st_audioConfig.u32_encodeDataAddr = MPE3_ENCODER_DATA_ADDR;
    st_audioConfig.u32_newPcmDataFlagAddr = SRAM_MODULE_SHARE_AUDIO_PCM;
    st_audioConfig.u8_channel = 2;
    HAL_MP3EncodePcmInit(&st_audioConfig, 0);

    st_ARCastStatus.u8_ARStatus &= (~(1 << ARCAST_COMMAND_SKY_FORMAT));
}

static void rcvFormatHandler_sky(void *p)
{
    uint32_t u32_rcvLen = 0;
    uint8_t  u8_recData[64];
    uint8_t  u8_ack;
    HAL_BB_UartComReceiveMsg(BB_UART_COM_SESSION_3, u8_recData, 64, &u32_rcvLen);

    if (1 == u32_rcvLen)
    {
        switch (u8_recData[0])
        {
            case ARCAST_COMMAND_SKY_FORMAT:
                {
                    st_ARCastStatus.u8_ARStatus |= 1 << ARCAST_COMMAND_SKY_FORMAT;
                    st_ARCastStatus.u8_ARStatus |= 1 << ARCAST_COMMAND_SKY_REQUEST_CAPABILITY;
                    dlog_info("rec gnd ack FORMAT");
                }
                break;
            case  ARCAST_COMMAND_SKY_AVDATA:
                {
                    if ((0 == (st_ARCastStatus.u8_ARStatus & (1 << ARCAST_SKY_ENCODER))))
                    {
                        STRU_SysEvent_H264InputFormatChangeParameter p;
                        p.index = 1;
                        p.width = s_st_ARCastSupportedOutputFormat[st_ARCastStatus.u8_AVFormat[0]][0];
                        p.hight = s_st_ARCastSupportedOutputFormat[st_ARCastStatus.u8_AVFormat[0]][1];
                        p.framerate = s_st_ARCastSupportedOutputFormat[st_ARCastStatus.u8_AVFormat[0]][2];
                        SYS_EVENT_Notify(SYS_EVENT_ID_H264_INPUT_FORMAT_CHANGE, (void*)&p);
                        st_ARCastStatus.u8_ARStatus |= 1 << ARCAST_SKY_ENCODER;
                        dlog_info("start send video %d %d %d %d",p.width, p.hight, p.framerate, st_ARCastStatus.u8_AVFormat[0]);
                    }

                    u8_ack = ARCAST_COMMAND_SKY_AVDATA;
                    HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_3, &u8_ack, 1);
                    dlog_info("rec gnd ack AVdata");
                }
        }
    }
    else if (20 == u32_rcvLen)
    {
        memcpy(&st_ARCastStatus.st_avCapability, u8_recData, u32_rcvLen);
        st_ARCastStatus.u8_ARStatus |= 1 << ARCAST_COMMAND_GND_CAPABILITY;
        st_ARCastStatus.u8_ARStatus |= 1 << ARCAST_COMMAND_SKY_REQUEST_CAPABILITY;
        u8_ack = ARCAST_COMMAND_GND_CAPABILITY;
        HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_3, &u8_ack, 1);
        dlog_info("rec gnd ack CAPABILITY");

        STRU_SysEvent_H264InputFormatChangeParameter p;
        p.index = 1;
        p.width = 0;
        p.hight = 0;
        p.framerate = 0;
        SYS_EVENT_Notify(SYS_EVENT_ID_H264_INPUT_FORMAT_CHANGE, (void*)&p);
        st_ARCastStatus.u8_ARStatus &= (~(1 << ARCAST_COMMAND_SKY_FORMAT));
        st_ARCastStatus.u8_ARStatus &= (~(1 << ARCAST_SKY_ENCODER));

    }
    else
    {
        dlog_error("unkonwn Command %d %d", u32_rcvLen, u8_recData[0]);
    }
}

uint8_t Command_FormatStatus(void)
{
    
    if (0 == (st_ARCastStatus.u8_ARStatus & (1 << ARCAST_COMMAND_SKY_REQUEST_CAPABILITY)))
    {

        dlog_info("power on request capability");
        uint8_t u8_command = ARCAST_COMMAND_SKY_REQUEST_CAPABILITY;
        HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_3, &u8_command, 1);
        HAL_Delay(1000);
        return ARCAST_SKY_FORMAT_CHANGE;
    }
    else if (st_ARCastStatus.u8_ARStatus & (1 << ARCAST_COMMAND_GND_CAPABILITY))
    {
        st_ARCastStatus.u8_ARStatus &= (~(1 << ARCAST_COMMAND_GND_CAPABILITY));
        if (st_ARCastStatus.st_avCapability.u8_VideoVidCount != 0xff)
        {
            //to hdp control
            create_eedid(Default_Edid_Block, &st_ARCastStatus.st_avCapability);
            it66021_init();
            return ARCAST_SKY_FORMAT_CHANGE;
        }
        else
        {
            return ARCAST_SKY_FORMAT_CHANGE;
        }
    }
    else if (0 == (st_ARCastStatus.u8_ARStatus & (1 << ARCAST_COMMAND_SKY_FORMAT)))
    {
        
        HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_3, (uint8_t*)(&st_ARCastStatus.u8_AVFormat), 2);
        HAL_Delay(1000);
        return ARCAST_SKY_FORMAT_CHANGE;
    }
    else
    {
        return ARCAST_SKY_FORMAT_STABLE;
    }
}

void Common_AVFORMATSysEventSKYInit(void)
{

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_H264_INPUT_FORMAT_CHANGE_ARCAST, Common_AVFORMAT_VideoSysEventCallBack);
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_AUDIO_INPUT_CHANGE, Common_AVFORMAT_AudioSysEventCallBack);
    
    HAL_BB_UartComRemoteSessionInit();        
    HAL_BB_UartComRegisterSession(BB_UART_COM_SESSION_3,
                                  BB_UART_SESSION_PRIORITY_HIGH,
                                  BB_UART_SESSION_DATA_NORMAL,
                                  rcvFormatHandler_sky);

    st_ARCastStatus.u8_ARStatus |= 1 << ARCAST_COMMAND_SKY_FORMAT;

   // st_ARCastStatus.u8_ARStatus = 1;
}



void ATM_StatusCallBack(uint8_t *st_status, uint32_t data_len)
{

}
