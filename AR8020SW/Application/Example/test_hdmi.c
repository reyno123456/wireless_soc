#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debuglog.h"
#include "hal_hdmi_rx.h"
#include "hal_adc.h"
#include "hal_i2c.h"
#include "it_typedef.h"
#include "it6602.h"
#include "it_66021.h"

static unsigned char hdmirxrd( unsigned char RegAddr)
{
    return IT_66021_ReadByte(0x92,RegAddr);
}

static unsigned char hdmirxwr( unsigned char RegAddr,unsigned char DataIn)
{    
    return IT_66021_WriteByte(0x92, RegAddr, DataIn);
}

static unsigned char  hdmirxset( unsigned char  offset, unsigned char  mask, unsigned char  ucdata )
{
    unsigned char  temp;
    temp = hdmirxrd(offset);
    temp = (temp&((~mask)&0xFF))+(mask&ucdata);
    return hdmirxwr(offset, temp);
}

void command_hdmi(void)
{
    uint16_t u16_width;
    uint16_t u16_hight;
    uint8_t u8_framerate;
    uint32_t u32_sampleRate;

    HAL_HDMI_RX_GetAudioSampleRate(HAL_HDMI_RX_1, &u32_sampleRate);
    HAL_HDMI_RX_GetVideoFormat(HAL_HDMI_RX_1, &u16_width, &u16_hight, &u8_framerate);
    dlog_info("video width=%d u16_hight=%d u8_framerate=%d ", u16_width, u16_hight, u8_framerate);
    dlog_info("audio sampleRate=%d ", u32_sampleRate);
    get_vid_info();
    show_vid_info();
}

static void chgbank( int bank )
{
    switch( bank ) {
    case 0 :
         hdmirxset(0x0F, 0x03, 0x00);
         break;
    case 1 :
         hdmirxset(0x0F, 0x03, 0x01);
         break;
    case 2 :
         hdmirxset(0x0F, 0x03, 0x02);
         break;
    case 3:
         hdmirxset(0x0F, 0x03, 0x03);
         break;
    default :
         break;
    }
}

void command_dump(void)
{
    uint8_t dump[0xFF]={0};
    uint8_t dump2[2]={0,0};
    uint8_t i = 0;
//    HAL_I2C_MasterInit(HAL_I2C_COMPONENT_2, 0x92 >> 1, HAL_I2C_STANDARD_SPEED);

    chgbank(0);
    printf("page0"); 
    for (i = 0; i < 0xFF; i++)
    {        
        printf("reg%02x=%02x\n",i,hdmirxrd(i)); 
        dlog_output(1000);
    }

    chgbank(1);
    printf("page1");
    for (i = 0; i < 0xFF; i++)
    {        
        printf("reg1%02x=%02x\n",i,hdmirxrd(i));
        dlog_output(100);
    }

    chgbank(2);
    printf("page2");
    for (i = 0; i < 0xFF; i++)
    {        
        printf("reg2%02x=%02x\n",i,hdmirxrd(i));
        dlog_output(100); 
    }


}


void command_hdmiHandler(uint8_t *index)
{
    uint32_t u32_index = strtoul(index, NULL, 0);
    /*switch (u32_index)
    {
        case 0:
            IT6602_fsm();
            break;
        case 1:
            IT6602_Interrupt();
            break;
        case 2:
            IT6602_Interrupt();
            IT6602_fsm();
            break;
        case 3:
            command_hdmi();
            break;
        case 4:
            command_dump();
            break;
    }*/
    //HDMI_RX_IdleCallback1(NULL);
    command_hdmi();
    dlog_info("command_hdmiHandler");
}
