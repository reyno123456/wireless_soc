#include "debuglog.h"
#include "command.h"
#include "hal_sram.h"
#include "cmsis_os.h"
#include "sys_event.h"
#include "upgrade.h"
#include "hal.h"
#include "hal_bb.h"
#include "test_usbh.h"
#include "hal_usb_otg.h"
#include "hal_sys_ctl.h"
#include "wireless_interface.h"
#include "hal_nv.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "arcast_appcommon.h"
#include "hal_pmu.h"
#include "minimp3.h"
#include <stdlib.h>
#include <string.h>
#include "test_usbh.h"
#include "systicks.h"
#include "hal_usb_host.h"
#include "hal_usb_device.h"
#include "hal_sram.h"
#include "hal_dma.h"

void Command_BBSendCommand(void const *argument);

void CONSOLE_Init(void)
{
    dlog_init(command_run, NULL, DLOG_CLIENT_PROCESSOR);
}


static void GenericInitial(void const *argument)
{
    //HAL_PMU_Init();
    Common_AVFORMATSysEventGroundInit();
    vTaskDelete(NULL);
}

mp3_decoder_t g_p_mp3Decoder = NULL;
#define MP3_BUFF_SIZE               (4096)
#define MP3_USERDATA_LEN            (8)
signed short u8_pcmDataArray[2][2304];

static void Mp3Decoder(void const *argument)
{
    uint8_t *p_mp3DataBuff = (uint8_t *)malloc(MP3_BUFF_SIZE+512);
    uint8_t *p_mp3DataBuffTmp = NULL;
    uint8_t *p_PCMDataBuff = NULL;
    uint16_t u16_mp3BuffLen = 0;
    int32_t  s32_decoderBuffLen = 0;
    uint32_t u32_decoderBuffpos = 0;
    uint32_t tmp = 0;
    uint32_t check = 0;
    uint32_t         src;
    uint32_t         dest;
    uint32_t         i,k;
    uint16_t         dataLenTemp;
    uint8_t   buff_index = 0;
    int frame_size = 0;
    uint8_t          u8_swap;
    uint8_t u8_checkSum = 0;;
    uint32_t u32_timeStamp = 0;;

    mp3_info_t mp3_info;
    p_mp3DataBuffTmp = p_mp3DataBuff;
    dlog_info("u8_pcmDataArray 0=%p 1=%p",u8_pcmDataArray[0] ,u8_pcmDataArray[1]);
    while(1)
    {
        u16_mp3BuffLen = HAL_SRAM_GetMp3BufferLength();
        if (u16_mp3BuffLen > 0)
        {

            if ((s32_decoderBuffLen + u16_mp3BuffLen) > (MP3_BUFF_SIZE + 512))
            {
                dlog_error("mp3 buff overflow :%d", s32_decoderBuffLen);
                HAL_SRAM_GetMp3Data(MP3_BUFF_SIZE, p_mp3DataBuffTmp);            
                s32_decoderBuffLen = MP3_BUFF_SIZE;
            }
            else
            {
                HAL_SRAM_GetMp3Data(u16_mp3BuffLen, (p_mp3DataBuffTmp + s32_decoderBuffLen));            
                s32_decoderBuffLen += u16_mp3BuffLen;
            }
            do
            {
                frame_size = mp3_decode(g_p_mp3Decoder, p_mp3DataBuffTmp, s32_decoderBuffLen, u8_pcmDataArray[buff_index], &mp3_info);                
                if (frame_size > 0)
                {
                    
                    if (mp3_info.audio_bytes < 0)
                    {
                        dlog_error("audio_bytes error :%d frame_size=%d", mp3_info.audio_bytes, frame_size);
                    }
                    else
                    {
                        k++;
                        if (k>100)
                        {
                            
                            u8_checkSum = 0;
                            u8_checkSum += ((uint8_t)p_mp3DataBuffTmp[frame_size+0]+(uint8_t)p_mp3DataBuffTmp[frame_size+1]+
                                            (uint8_t)p_mp3DataBuffTmp[frame_size+2]+(uint8_t)p_mp3DataBuffTmp[frame_size+4]+
                                            (uint8_t)p_mp3DataBuffTmp[frame_size+5]+(uint8_t)p_mp3DataBuffTmp[frame_size+6]+
                                            (uint8_t)p_mp3DataBuffTmp[frame_size+7]);
                            if (u8_checkSum != p_mp3DataBuffTmp[frame_size+3])
                            {
                                dlog_error("timestamp error stream=%x checkSum=%x", (uint8_t)p_mp3DataBuffTmp[frame_size+3], u8_checkSum);
                                dlog_error("stream data=%x %x %x %x %x %x %x %x", (uint8_t)p_mp3DataBuffTmp[frame_size+0],(uint8_t)p_mp3DataBuffTmp[frame_size+1],
                                                                                  (uint8_t)p_mp3DataBuffTmp[frame_size+2],(uint8_t)p_mp3DataBuffTmp[frame_size+4],
                                                                                  (uint8_t)p_mp3DataBuffTmp[frame_size+5],(uint8_t)p_mp3DataBuffTmp[frame_size+6],
                                                                                  (uint8_t)p_mp3DataBuffTmp[frame_size+7],(uint8_t)p_mp3DataBuffTmp[frame_size+3]);
                            }
                            else
                            {
                                k=0;
                                u32_timeStamp |= (p_mp3DataBuffTmp[frame_size+4]&0xff)<<24;
                                u32_timeStamp |= (p_mp3DataBuffTmp[frame_size+5]&0xff)<<16;
                                u32_timeStamp |= (p_mp3DataBuffTmp[frame_size+6]&0xff)<<8;
                                u32_timeStamp |= (p_mp3DataBuffTmp[frame_size+7])&0xff;
                                dlog_info("time stamp =%x",u32_timeStamp);
                                u32_timeStamp = 0;
                                HAL_USB_CustomerSendData((p_mp3DataBuffTmp + frame_size), MP3_USERDATA_LEN, 0);
                            }                            
                        }
                        
                        p_PCMDataBuff = (uint8_t *)(u8_pcmDataArray[buff_index]);
                    
                        for (i = 0; i < mp3_info.audio_bytes ; i+=4)
                        {
                            u8_swap                = p_PCMDataBuff[i];
                            p_PCMDataBuff[i]       = p_PCMDataBuff[i+3];
                            p_PCMDataBuff[i+3]     = u8_swap;

                            u8_swap                = p_PCMDataBuff[i+1];
                            p_PCMDataBuff[i+1]     = p_PCMDataBuff[i+2];
                            p_PCMDataBuff[i+2]     = u8_swap;
                        }
                                        
                        if (HAL_OK != HAL_USB_AudioDataSend(p_PCMDataBuff, mp3_info.audio_bytes, 0))
                        {
                            dlog_error("send audio data timeout");
                        }

                        buff_index++;
                        buff_index &= 1;
                    }
                    if (s32_decoderBuffLen < (frame_size+MP3_USERDATA_LEN))
                    {
                        dlog_error("mp3 data error s32_decoderBuffLen=%d",s32_decoderBuffLen);
                        s32_decoderBuffLen = 0;
                        k = 100;
                        break;
                    }
                    p_mp3DataBuffTmp += (frame_size+MP3_USERDATA_LEN);
                    s32_decoderBuffLen -= (frame_size+MP3_USERDATA_LEN);
                }
                else if (s32_decoderBuffLen > 500)
                {
                    p_mp3DataBuffTmp += (s32_decoderBuffLen-500);
                    s32_decoderBuffLen = 500;
                    k = 100;
                }
            }while((frame_size > 0) && (s32_decoderBuffLen > MP3_USERDATA_LEN));
            
            memcpy(p_mp3DataBuff, p_mp3DataBuffTmp, s32_decoderBuffLen);
            p_mp3DataBuffTmp = p_mp3DataBuff;

        }
        HAL_Delay(1);        
    }
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
    dlog_set_output_level(LOG_LEVEL_INFO);
    /* initialize the uart */
    CONSOLE_Init();
	
    dlog_info("cpu0 start!!!\n");	

    g_p_mp3Decoder = mp3_create();
	
    HAL_GPIO_InPut(HAL_GPIO_NUM99);

    HAL_USB_ConfigPHY();

    HAL_USB_InitOTG(HAL_USB_PORT_0);

    HAL_USB_InitOTG(HAL_USB_PORT_1);

    HAL_SRAM_ReceiveVideoConfig();

    HAL_SRAM_ChannelConfig(ENUM_HAL_SRAM_CHANNEL_TYPE_AUDIO,
                           HAL_USB_PORT_0,
                           0);

    HAL_SRAM_ChannelConfig(ENUM_HAL_SRAM_CHANNEL_TYPE_VIDEO0,
                           HAL_USB_PORT_0,
                           1);

    HAL_DMA_init();

    HAL_NV_Init();
	
    osThreadDef(GenericInitialTask, GenericInitial, osPriorityHigh, 0, 4 * 128);
    osThreadCreate(osThread(GenericInitialTask), NULL);

    osThreadDef(USBHStatus_Task, USBH_USBHostStatus, osPriorityNormal, 0, 4 * 128);
    osThreadCreate(osThread(USBHStatus_Task), NULL);
    
    osThreadDef(MP3_Task, Mp3Decoder, osPriorityBelowNormal, 0, 6 * 1024);
    osThreadCreate(osThread(MP3_Task), NULL);
     
    osThreadDef(SendCommand_Task, Command_BBSendCommand, osPriorityBelowNormal, 0, 4 * 128);
    osThreadCreate(osThread(SendCommand_Task), NULL);

    osThreadDef(IOTask, IO_Task, osPriorityIdle, 0, 16 * 128);
    osThreadCreate(osThread(IOTask), NULL);

    Wireless_TaskInit(WIRELESS_USE_RTOS);
    osKernelStart();

    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
    }
}

