#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hal_ret_type.h"
#include "hal_encodemp3.h"
#include "hal_softi2s.h"
#include "debuglog.h"
#include "systicks.h"
#include "cpu_info.h"
#include "dma.h"
#include "hal_sram.h"
#include "layer3.h"
#include "rtc.h" 
#include "memory_config.h" 
   
shine_t        st_s;
uint32_t u32_rawDataLenght;
uint32_t u32_rawDataAddr;
uint32_t u32_encodeDataAddr;
int32_t s32_samples_per_pass;
uint32_t u32_frameSize;
volatile uint32_t *pu32_newPcmDataFlagAddr;
volatile uint32_t g_u32_dstAddress=MPE3_ENCODER_DATA_ADDR;
uint32_t g_u32_audioBypassAddr=0;


HAL_BOOL_T HAL_MP3EncodePcmInit(const STRU_MP3_ENCODE_CONFIGURE_WAVE *st_mp3EncodeConfg, uint8_t dataPath)
{          
    shine_config_t st_config; 
    pu32_newPcmDataFlagAddr=(uint32_t *)(st_mp3EncodeConfg->u32_newPcmDataFlagAddr);
    (*pu32_newPcmDataFlagAddr) = 0;

    u32_rawDataLenght = (st_mp3EncodeConfg->u32_rawDataLenght);
    u32_rawDataAddr = st_mp3EncodeConfg->u32_rawDataAddr;
    u32_encodeDataAddr = st_mp3EncodeConfg->u32_encodeDataAddr;
    shine_set_config_mpeg_defaults(&(st_config.mpeg));

    st_config.wave.channels = st_mp3EncodeConfg->u8_channel;
    st_config.wave.samplerate = st_mp3EncodeConfg->e_samplerate;
    st_config.mpeg.mode = st_mp3EncodeConfg->e_modes;
    st_s = shine_initialise(&st_config);
    if (NULL == st_s)
    {
        return HAL_FALSE;
    }

    s32_samples_per_pass = shine_samples_per_pass(st_s);
    u32_frameSize = sizeof(short int) * s32_samples_per_pass * st_config.wave.channels; 

    if (dataPath == 0)
    {
        HAL_SRAM_EnableSkyBypassVideo(HAL_SRAM_VIDEO_CHANNEL_0);
        g_u32_audioBypassAddr = AUDIO_BYPASS_START_CH0;
    }
    else
    {
        HAL_SRAM_EnableSkyBypassVideo(HAL_SRAM_VIDEO_CHANNEL_1);
        g_u32_audioBypassAddr = AUDIO_BYPASS_START_CH1;
    }

    dlog_info("encode mp3 init  %x %x %x %d\n", u32_rawDataLenght,u32_rawDataAddr,u32_encodeDataAddr,u32_frameSize);
    return  HAL_TRUE;
}

HAL_BOOL_T HAL_MP3EncodePcmUnInit(void)
{
    shine_close(st_s);
    dlog_info("encode mp3 uninit");
}

static int hal_mp3_check_header(uint32_t header){
    /* header */
    if ((header & 0xffe00000) != 0xffe00000)
        return -1;
    /* layer check */
    if ((header & (3<<17)) != (1 << 17))
        return -1;
    /* bit rate */
    if ((header & (0xf<<12)) == 0xf<<12)
        return -1;
    /* frequency */
    if ((header & (3<<10)) == 3<<10)
        return -1;
    return 0;
}

void HAL_MP3EncodePcm(void)
{   
    if (0 != (*pu32_newPcmDataFlagAddr))
    {
        uint32_t u32_tmpRawDataLenght = u32_rawDataLenght;
        uint32_t u32_tmpEncodeDataAddr = u32_encodeDataAddr+(AUDIO_DATA_BUFF_COUNT*u32_frameSize)*(*pu32_newPcmDataFlagAddr-1);
        uint32_t u32_tmpRawDataAddr = u32_rawDataAddr+AUDIO_DATA_BUFF_SIZE*(*pu32_newPcmDataFlagAddr-1);
        int s32_encodeLenght = 0;
        uint8_t  *pu8_data = NULL;
        uint16_t *pu16_rawDataAddr = (uint16_t *)(u32_tmpRawDataAddr);
        uint8_t  *pu8_encodeDataAddr = (uint8_t *)(u32_tmpEncodeDataAddr);
        volatile uint32_t tick=0;
        uint32_t i=0,header=0,extra_bytes=0,j=0;
        uint8_t ch[AUDIO_DATA_BUFF_COUNT*420]={0};
        memset(ch, 0, AUDIO_DATA_BUFF_COUNT*420);
        uint8_t g_u8_userDataArray[8]={0};
        
        
        while (u32_tmpRawDataLenght)
        {
            
            pu8_data = shine_encode_buffer_interleaved(st_s, pu16_rawDataAddr, &s32_encodeLenght);                   
            u32_tmpRawDataLenght -= u32_frameSize;                     
            u32_tmpRawDataAddr+= u32_frameSize;            
            pu16_rawDataAddr = (uint16_t *)(u32_tmpRawDataAddr);
            memcpy(&ch[i],pu8_data,s32_encodeLenght);
            i+=s32_encodeLenght;
        }
        
		tick =  *((volatile uint32_t *)(SRAM_MODULE_SHARE_AVSYNC_TICK));
        g_u8_userDataArray[0] = 0x35;
        g_u8_userDataArray[1] = 0x53;
        g_u8_userDataArray[2] = 0x55;
        g_u8_userDataArray[4] = (tick>>24)&0xff;
        g_u8_userDataArray[5] = (tick>>16)&0xff;
        g_u8_userDataArray[6] = (tick>>8)&0xff;
        g_u8_userDataArray[7] = (tick)&0xff;

        g_u8_userDataArray[3] += ((uint8_t)g_u8_userDataArray[0]+(uint8_t)g_u8_userDataArray[1]+
                                  (uint8_t)g_u8_userDataArray[2]+(uint8_t)g_u8_userDataArray[4]+
                                  (uint8_t)g_u8_userDataArray[5]+(uint8_t)g_u8_userDataArray[6]+
                                  (uint8_t)g_u8_userDataArray[7]);
#if 1   
        extra_bytes = 0;
        for (j = 0; j < i; j++)
        {
            header = (ch[extra_bytes] << 24) | (ch[extra_bytes+1] << 16) | (ch[extra_bytes+2] << 8) | ch[extra_bytes+3];
            if(hal_mp3_check_header(header) < 0)
            {
                extra_bytes++;        
            }
            else
            {
                //dlog_info("extra_bytes %d %x",extra_bytes, header);
                break;
            }
        }

        memcpy((uint8_t *)g_u32_audioBypassAddr, ch, extra_bytes+1);
        memcpy((uint8_t *)g_u32_audioBypassAddr, g_u8_userDataArray, 8);     
        memcpy((uint8_t *)g_u32_audioBypassAddr, &ch[extra_bytes], i-extra_bytes-1);
#else
        memcpy((uint8_t *)g_u32_audioBypassAddr, ch, i);                  
#endif
	//dlog_info("encode mp3 ok %x\n", tick);
        
        g_u32_dstAddress+=(i+8);
        (*pu32_newPcmDataFlagAddr) = 0;

    }    

}

