#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "data_type.h"
#include "memory_config.h"
#include "it_define.h"
#include "i2c.h"
#include "it_66021.h"
#include "debuglog.h"
#include "sys_event.h"
#include "systicks.h"
#include "interrupt.h"

#include "config.h"
#include "Utility.h"
#include "it6602.h"
#include "it6602_reg.h"

extern volatile VTiming CurVTiming;

typedef unsigned char (*HDMI_SET_TABLE)[3];
static HDMI_SET_TABLE hdmi_edid_table = NULL;

static unsigned int s_u8Array_ARCastSupportedOutputFormat[][4] =
{
    { 720, 480,  50,  2},//4:3
    { 720, 480,  60,  3},//16:9
    { 720, 576,  50, 17},//4:3
    { 720, 576,  60, 18},//16:9
    {1280, 720,  60,  4},//16:9
    {1280, 720,  50, 19},//16:9
    {1920, 1080, 30, 34},//16:9
};

void IT_Delay(uint32_t delay)
{
    if (40*delay>0xffffffff)
    {
        for(uint32_t i = 0; i < 0xffffffff; i++)
        {
            ;
        }
    }
    else
    {
        for(uint32_t i = 0; i < 40*delay; i++)
        {
            ;
        }        
    }
}

uint8_t IT_66021_WriteByte(uint8_t slv_addr, uint8_t sub_addr, uint8_t val)
{
    uint8_t flag;
    uint8_t data[2] = {sub_addr, val};
    I2C_Master_WaitTillIdle(IT_66021_I2C_COMPONENT_NUM, IT_66021_I2C_I2C_MAX_DELAY_MS);
    flag = I2C_Master_WriteData(IT_66021_I2C_COMPONENT_NUM, slv_addr >> 1, data, 2);
    if(flag==0)
    {
        dlog_info("=====  Write Reg0x%X=%X data error=====  \n",sub_addr,val);
        return flag;
    }
    I2C_Master_WaitTillIdle(IT_66021_I2C_COMPONENT_NUM, IT_66021_I2C_I2C_MAX_DELAY_MS);
    if(flag==0)
    {
        dlog_info("=====  Write Reg0x%X=%X timeout=====  \n",sub_addr,val);
        return flag;
    }
    return 0;
}


uint8_t IT_66021_ReadByte(uint8_t slv_addr, uint8_t sub_addr)
{
    uint8_t sub_addr_tmp = sub_addr;
    uint8_t val = 0;
    I2C_Master_WaitTillIdle(IT_66021_I2C_COMPONENT_NUM, IT_66021_I2C_I2C_MAX_DELAY_MS);
    I2C_Master_ReadData(IT_66021_I2C_COMPONENT_NUM, slv_addr >> 1, &sub_addr_tmp, 1, &val, 1);
    I2C_Master_WaitTillIdle(IT_66021_I2C_COMPONENT_NUM, IT_66021_I2C_I2C_MAX_DELAY_MS);
    return val;
}

uint8_t IT_66021_WriteBytes(uint8_t slv_addr, uint8_t sub_addr, uint8_t byteno, uint8_t *p_data)
{
    uint8_t flag;
    uint8_t *pdata = malloc(byteno+1);
    if (NULL != pdata)
    {
        (*pdata) = sub_addr;
        memcpy(pdata+1, p_data, byteno);
        if( byteno>0 )
        {
            I2C_Master_WaitTillIdle(IT_66021_I2C_COMPONENT_NUM, IT_66021_I2C_I2C_MAX_DELAY_MS);
			flag = I2C_Master_WriteData(IT_66021_I2C_COMPONENT_NUM, slv_addr >> 1, pdata, byteno+1);
        }        
        if(flag==0)
        {
            dlog_info("=====  Write Reg0x%X=%X data error=====  \n",sub_addr,(int)p_data);
            free(pdata);
            return flag;
        }
        I2C_Master_WaitTillIdle(IT_66021_I2C_COMPONENT_NUM, IT_66021_I2C_I2C_MAX_DELAY_MS);
        //FIX_ID_002 xxxxx
        if(flag==0)
        {
            dlog_info("=====  Write Reg0x%X=%X timeout=====  \n",sub_addr,(int)p_data);
            free(pdata);
            return flag;
        }
        free(pdata);
    }
    return 0;
}

void IT_66021_Set(unsigned char slv_addr, unsigned char sub_addr, unsigned char mask, unsigned char val)
{
    
    unsigned char sub_addr_tmp = sub_addr;
    unsigned char tmpal = 0;
    I2C_Master_ReadData(IT_66021_I2C_COMPONENT_NUM, slv_addr >> 1, &sub_addr_tmp, 1, &tmpal, 1);

    unsigned char data[2] = {sub_addr, ((tmpal&((~mask)&0xFF)))+(mask&val)};
    I2C_Master_WriteData(IT_66021_I2C_COMPONENT_NUM, slv_addr >> 1, data, 2);
}

static void IT_66021_I2CInitial(void)
{
    static uint8_t i2c_initialized = 0;
    if (i2c_initialized == 0)
    {
        I2C_Init(IT_66021_I2C_COMPONENT_NUM, I2C_Master_Mode, RX_I2C_IO_MAP_ADDR >> 1, I2C_Standard_Speed);
        INTR_NVIC_SetIRQPriority(I2C_INTR2_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,4,0));
        reg_IrqHandle(I2C_INTR2_VECTOR_NUM, I2C_Master_IntrSrvc, NULL);
        INTR_NVIC_EnableIRQ(I2C_INTR2_VECTOR_NUM);
        msleep(100);
        i2c_initialized = 1;
    }
}

uint8_t IT_66021_GetVideoFormat(uint8_t index, uint16_t* widthPtr, uint16_t* hightPtr, uint8_t* framteratePtr, uint8_t* vic)
{
    if (TRUE == IsVideoOn())
    {
        uint8_t i = 0;
        uint8_t array_size = sizeof(s_u8Array_ARCastSupportedOutputFormat)/sizeof(s_u8Array_ARCastSupportedOutputFormat[0]);
        struct it6602_dev_data *it6602data = get_it6602_dev_data();
        for (i = 0; i < array_size; i++)
        {
            if (it6602data->VIC == s_u8Array_ARCastSupportedOutputFormat[i][3])
            {
                *widthPtr = s_u8Array_ARCastSupportedOutputFormat[i][0];
                *hightPtr = s_u8Array_ARCastSupportedOutputFormat[i][1];
                *framteratePtr = s_u8Array_ARCastSupportedOutputFormat[i][2];
                *vic = it6602data->VIC;
                return TRUE;
            }
        }
        
        uint8_t hdmi_i2c_addr = (index == 0) ? RX_I2C_HDMI_MAP_ADDR : (RX_I2C_HDMI_MAP_ADDR + 2);

        uint32_t u32_HTotal   = (((IT_66021_ReadByte(hdmi_i2c_addr, 0x9D))&0x3F)<<8) + IT_66021_ReadByte(hdmi_i2c_addr, 0x9C);
        uint32_t u32_HActive  = (((IT_66021_ReadByte(hdmi_i2c_addr, 0x9F))&0x3F)<<8) + IT_66021_ReadByte(hdmi_i2c_addr, 0x9E);

        uint32_t u32_VTotal   = (((IT_66021_ReadByte(hdmi_i2c_addr, 0xA4))&0x0F)<<8) + IT_66021_ReadByte(hdmi_i2c_addr, 0xA3);
        uint32_t u32_VActive  = (((IT_66021_ReadByte(hdmi_i2c_addr, 0xA4))&0xF0)<<4) + IT_66021_ReadByte(hdmi_i2c_addr, 0xA5);

        uint8_t u8_rddata = IT_66021_ReadByte(hdmi_i2c_addr, 0x9A);
        uint32_t PCLK = (124*255/u8_rddata)/10;
        uint64_t u64_FrameRate = (uint64_t)(PCLK)*1000*1000;
        u64_FrameRate /= u32_HTotal;
        u64_FrameRate /= u32_VTotal;
         
        *widthPtr = (uint16_t)u32_HActive;
        *hightPtr = (uint16_t)u32_VActive;
       
        if ((u64_FrameRate > 55) || (u64_FrameRate > 65))   
        {
            *framteratePtr = 60;
        }
        else if ((u64_FrameRate > 45) || (u64_FrameRate > 55))
        {
            *framteratePtr = 50;    
        }
        else if ((u64_FrameRate > 25) || (u64_FrameRate > 35))
        {
            *framteratePtr = 30;
        }
        *vic = 0xff;
        return TRUE;
    }
    else
    {
        *widthPtr = 0;
        *hightPtr = 0;
        *framteratePtr = 0;
        *vic = 0xff;
        return FALSE;
    }
}

uint8_t IT_66021_GetAudioSampleRate(uint8_t index, uint32_t* sampleRate)
{
    struct it6602_dev_data *it6602data = get_it6602_dev_data();
    if (ASTATE_AudioOn == it6602data->m_AState)
    {
        *sampleRate = (int)it6602data->m_RxAudioCaps.SampleFreq;
        return TRUE;
    }
    else
    {
        *sampleRate = 1;
        return FALSE;
    }
}


uint8_t IT_66021_IrqHandler0(void)
{
        IT6602_Interrupt();
        IT6602_fsm();    
}

uint8_t IT_66021_IrqHandler1(void)
{
    IT6602_Interrupt();
    IT6602_fsm(); 
    //IT6602_fsm();
}


void InitMessage()
{
    dlog_info("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
#if defined(_IT6602_)
    dlog_info("           IT6602 \n");
#elif defined(_IT66023_)
    dlog_info("           IT66023 \n");
#else
    dlog_info("           IT66021 \n");
#endif
    dlog_info("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");

}

void IT_66021_Initial(uint8_t index)
{
    InitMessage();
    IT_66021_I2CInitial();
    it66021_init();

}
