#include <stddef.h>
#include <stdint.h>
#include "data_type.h"
#include "memory_config.h"
#include "adv_define.h"
#include "i2c.h"
#include "adv_7611.h"
#include "debuglog.h"
#include "sys_event.h"
#include "interrupt.h"


typedef unsigned char (*HDMI_SET_TABLE)[3];
static HDMI_SET_TABLE hdmi_edid_table = NULL;

static unsigned char hdmi_default_settings[][3] =
{
    /* Default settings: 1080p, SAV and EAV.
    /* 1080p Any Color Space In (YCrCb 444 24bit from ADV761x) Through HDMI Out 444 YCrCb VIC[16,31,32]: */
    {0x98, 0x01, 0x06},    //Prim_Mode =110b HDMI-GR
    {0x98, 0x02, 0xF5},    //Auto CSC, YCrCb out, Set op_656 bit
    {0x98, 0x03, 0x80},    //16 bit SDR 422 Mode 0
    {0x98, 0x05, 0x2C},    //AV Codes Off
    {0x98, 0x06, 0xA0},    //Invert VS,HS pins with clock and DE
    {0x98, 0x0B, 0x44},    //Power up part
    {0x98, 0x0C, 0x42},    //Power up part
    {0x98, 0x14, 0x7F},    //Max Drive Strength
    {0x98, 0x15, 0x80},    //Disable Tristate of Pins
    {0x98, 0x19, 0x83},    //LLC DLL phase
    {0x98, 0x33, 0x40},    //LLC DLL enable
    {0x44, 0xBA, 0x00},    //Set HDMI FreeRun Disbale
    {0x64, 0x40, 0x81},    //Disable HDCP 1.1 features
    {0x68, 0x9B, 0x03},    //ADI recommended setting
    {0x68, 0xC1, 0x01},    //ADI recommended setting
    {0x68, 0xC2, 0x01},    //ADI recommended setting
    {0x68, 0xC3, 0x01},    //ADI recommended setting
    {0x68, 0xC4, 0x01},    //ADI recommended setting
    {0x68, 0xC5, 0x01},    //ADI recommended setting
    {0x68, 0xC6, 0x01},    //ADI recommended setting
    {0x68, 0xC7, 0x01},    //ADI recommended setting
    {0x68, 0xC8, 0x01},    //ADI recommended setting
    {0x68, 0xC9, 0x01},    //ADI recommended setting
    {0x68, 0xCA, 0x01},    //ADI recommended setting
    {0x68, 0xCB, 0x01},    //ADI recommended setting
    {0x68, 0xCC, 0x01},    //ADI recommended setting
    {0x68, 0x00, 0x00},    //Set HDMI Input Port A
    {0x68, 0x83, 0xFE},    //Enable clock terminator for port A
    {0x68, 0x6F, 0x0C},    //ADI recommended setting
    {0x68, 0x85, 0x1F},    //ADI recommended setting
    {0x68, 0x87, 0x70},    //ADI recommended setting
    {0x68, 0x8D, 0x04},    //LFG
    {0x68, 0x8E, 0x1E},    //HFG
    {0x68, 0x1A, 0x8A},    //unmute audio
    {0x68, 0x57, 0xDA},    //ADI recommended setting
    {0x68, 0x58, 0x01},    //ADI recommended setting
    {0x68, 0x03, 0x98},    //DIS_I2C_ZERO_COMPR
    {0x68, 0x75, 0x10},    //DDC drive strength
    {0x98, 0x40, 0x21},    //INT1 Drives low when active
    {0x98, 0x6E, 0x02},    //V_LOCKED
    {0xFF, 0xFF, 0xFF}     //End flag
};

static unsigned char adv_i2c_addr_table[][3] =
{
    //{0x98, 0xFF, 0x80},                       //I2C reset
    {0x98, 0xF4, RX_I2C_CEC_MAP_ADDR},          //CEC
    {0x98, 0xF5, RX_I2C_INFOFRAME_MAP_ADDR},    //INFOFRAME
    {0x98, 0xF8, RX_I2C_AFE_DPLL_MAP_ADDR},     //DPLL
    {0x98, 0xF9, RX_I2C_REPEATER_MAP_ADDR},     //KSV
    {0x98, 0xFA, RX_I2C_EDID_MAP_ADDR},         //EDID
    {0x98, 0xFB, RX_I2C_HDMI_MAP_ADDR},         //HDMI
    {0x98, 0xFD, RX_I2C_CP_MAP_ADDR},           //CP
    {0xFF, 0xFF, 0xFF}                          //End flag
};

static void ADV_7611_Delay(unsigned int count)
{
    volatile unsigned int i = count;
    while (i--);
}

static void ADV_7611_WriteByte(uint8_t slv_addr, uint8_t sub_addr, uint8_t val)
{
    unsigned char data[2] = {sub_addr, val};
    I2C_Master_WriteData(ADV_7611_I2C_COMPONENT_NUM, slv_addr >> 1, data, 2);
    I2C_Master_WaitTillIdle(ADV_7611_I2C_COMPONENT_NUM, ADV_7611_I2C_MAX_DELAY);
}

static uint8_t ADV_7611_ReadByte(uint8_t slv_addr, uint8_t sub_addr)
{
    unsigned char sub_addr_tmp = sub_addr;
    unsigned char val = 0;
    I2C_Master_ReadData(ADV_7611_I2C_COMPONENT_NUM, slv_addr >> 1, &sub_addr_tmp, 1, &val, 1);
    I2C_Master_WaitTillIdle(ADV_7611_I2C_COMPONENT_NUM, ADV_7611_I2C_MAX_DELAY);
    return val;
}

static void ADV_7611_I2CInitial(void)
{
    static uint8_t i2c_initialized = 0;
    if (i2c_initialized == 0)
    {
        I2C_Init(ADV_7611_I2C_COMPONENT_NUM, I2C_Master_Mode, RX_I2C_IO_MAP_ADDR >> 1, I2C_Fast_Speed);
        INTR_NVIC_SetIRQPriority(I2C_INTR2_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_I2C_DEFAULT,0));
        reg_IrqHandle(I2C_INTR2_VECTOR_NUM, I2C_Master_IntrSrvc, NULL);
        INTR_NVIC_EnableIRQ(I2C_INTR2_VECTOR_NUM);
        
        ADV_7611_Delay(100);
        ADV_7611_WriteByte(0x98, 0x1B, 0x01);
        ADV_7611_Delay(100);
        i2c_initialized = 1;
    }
}

#define MAX_TABLE_ITEM_COUNT 2000
static void ADV_7611_WriteTable(uint8_t index, unsigned char(*reg_table)[3])
{
    unsigned int i = 0;
    unsigned char slv_addr_offset = (index == 0) ? 0 : 2;

    if (reg_table == NULL)
    {
        dlog_error("reg_table is NULL", reg_table);
        return;
    }
    
    while (i < MAX_TABLE_ITEM_COUNT)
    {
        if ((reg_table[i][0] == 0xFF) && (reg_table[i][1] == 0xFF) && (reg_table[i][2] == 0xFF))
        {
            break;
        }

        if ((reg_table[i][0] == 0) && (reg_table[i][1] == 0) && (reg_table[i][2] == 0))
        {
            break;
        }

        if (adv_i2c_addr_table == reg_table)
        {
            ADV_7611_WriteByte(reg_table[i][0] + slv_addr_offset, reg_table[i][1], reg_table[i][2] + slv_addr_offset);
        }
        else
        {
            ADV_7611_WriteByte(reg_table[i][0] + slv_addr_offset, reg_table[i][1], reg_table[i][2]);
        }
        
        i++;
    }
}

static void ADV_7611_GenericInitial(uint8_t index)
{
    ADV_7611_WriteTable(index, adv_i2c_addr_table);
    ADV_7611_WriteTable(index, hdmi_edid_table);
    ADV_7611_Delay(1000);
    ADV_7611_WriteTable(index, hdmi_default_settings);
}

void ADV_7611_Initial(uint8_t index)
{
    STRU_SettingConfigure* cfg_addr;
    GET_CONFIGURE_FROM_FLASH(cfg_addr);    
    hdmi_edid_table = (HDMI_SET_TABLE)(&(cfg_addr->hdmi_configure));
    
    ADV_7611_I2CInitial();
    ADV_7611_GenericInitial(index);
    dlog_info("HDMI ADV7611 %d init finished!", index);
}

void ADV_7611_GetVideoFormat(uint8_t index, uint16_t* widthPtr, uint16_t* hightPtr, uint8_t* framteratePtr)
{
    uint16_t val1 = 0;
    uint16_t val2 = 0;
    uint16_t width = 0;
    uint16_t hight = 0;
    uint8_t frame_rate = 0;

    uint16_t bl_clk = 0;
    uint32_t hfreq = 0;
    uint16_t field0_hight = 0; 
    uint16_t field1_hight = 0;
    uint16_t field_hight = 0;
    uint32_t vfreq = 0;

    uint8_t hdmi_i2c_addr = (index == 0) ? RX_I2C_HDMI_MAP_ADDR : (RX_I2C_HDMI_MAP_ADDR + 2);
    uint8_t cp_i2c_addr = (index == 0) ? RX_I2C_CP_MAP_ADDR : (RX_I2C_CP_MAP_ADDR + 2);

    val1 = ADV_7611_ReadByte(hdmi_i2c_addr, 0x07) & 0x1F;
    val2 = ADV_7611_ReadByte(hdmi_i2c_addr, 0x08);
    width = (val1 << 8) + val2;
 
    val1 = ADV_7611_ReadByte(hdmi_i2c_addr, 0x09) & 0x1F;
    val2 = ADV_7611_ReadByte(hdmi_i2c_addr, 0x0a);
    hight = (val1 << 8) + val2;
    
    field0_hight = ((ADV_7611_ReadByte(hdmi_i2c_addr, 0x26) & 0x3f) << 8) | ADV_7611_ReadByte(hdmi_i2c_addr, 0x27);
    field1_hight = ((ADV_7611_ReadByte(hdmi_i2c_addr, 0x28) & 0x3f) << 8) | ADV_7611_ReadByte(hdmi_i2c_addr, 0x29);
    field_hight = (field0_hight + field1_hight) / 4;
    bl_clk = ((ADV_7611_ReadByte(cp_i2c_addr, 0xb1) & 0x3f) << 8) | ADV_7611_ReadByte(cp_i2c_addr, 0xb2);

    if ((field_hight != 0) && (bl_clk != 0))
    {
        hfreq = (ADV761x_CRYSTAL_CLK * 8) / bl_clk;
        vfreq = hfreq / field_hight;
        frame_rate = vfreq;
     }
     
     *widthPtr = width;
     *hightPtr = hight;
     *framteratePtr = frame_rate;
}

void ADV_7611_GetAudioSampleRate(uint8_t index, uint32_t* sampleRate)
{

    uint8_t hdmi_i2c_addr = (index == 0) ? RX_I2C_HDMI_MAP_ADDR : (RX_I2C_HDMI_MAP_ADDR + 2);

    *sampleRate = ADV_7611_ReadByte(hdmi_i2c_addr, 0x39) & 0xF;
}

void ADV_7611_DumpOutEdidData(uint8_t index)
{
    dlog_info("Edid Data:");

    unsigned char slv_addr_offset = (index == 0) ? 0 : 2; 

    unsigned int i;
    unsigned char val = 0;
    for (i = 0; ; i++)
    {
        if ((hdmi_edid_table[i][0] == 0xFF) && (hdmi_edid_table[i][1] == 0xFF) && (hdmi_edid_table[i][2] == 0xFF))
        {
            break;
        }

        if ((hdmi_edid_table[i][0] == 0) && (hdmi_edid_table[i][1] == 0) && (hdmi_edid_table[i][2] == 0))
        {
            break;
        }

        val = ADV_7611_ReadByte(hdmi_edid_table[i][0] + slv_addr_offset, hdmi_edid_table[i][1]);
        
        if (val == hdmi_edid_table[i][2])
        {
            dlog_info("0x%x, 0x%x, 0x%x", hdmi_edid_table[i][0] + slv_addr_offset, hdmi_edid_table[i][1], val);
        }
        else
        {
            dlog_info("0x%x, 0x%x, 0x%x, Error: right value 0x%x!", hdmi_edid_table[i][0] + slv_addr_offset, hdmi_edid_table[i][1], val, hdmi_edid_table[i][2]);
        }
    }
}

void ADV_7611_DumpOutDefaultSettings(uint8_t index)
{
    unsigned int i;
    unsigned char val = 0;
    unsigned char slv_addr_offset = (index == 0) ? 0 : 2; 

    dlog_info("I2C Address Table:");
    for (i = 1; ; i++)
    {
        if ((adv_i2c_addr_table[i][0] == 0xFF) && (adv_i2c_addr_table[i][1] == 0xFF) && (adv_i2c_addr_table[i][2] == 0xFF))
        {
            break;
        }

        if ((adv_i2c_addr_table[i][0] == 0) && (adv_i2c_addr_table[i][1] == 0) && (adv_i2c_addr_table[i][2] == 0))
        {
            break;
        }

        val = ADV_7611_ReadByte(adv_i2c_addr_table[i][0] + slv_addr_offset, adv_i2c_addr_table[i][1]);
        
        if (val == (adv_i2c_addr_table[i][2] + slv_addr_offset))
        {
            dlog_info("0x%x, 0x%x, 0x%x", adv_i2c_addr_table[i][0] + slv_addr_offset, adv_i2c_addr_table[i][1], val);
        }
        else
        {
            dlog_info("0x%x, 0x%x, 0x%x, Error: right value 0x%x!", adv_i2c_addr_table[i][0] + slv_addr_offset, adv_i2c_addr_table[i][1], val, adv_i2c_addr_table[i][2] + slv_addr_offset);            
        }
    }
 
    dlog_info("Default Settings:"); 
    for (i = 0; ; i++)
    {
        if ((hdmi_default_settings[i][0] == 0xFF) && (hdmi_default_settings[i][1] == 0xFF) && (hdmi_default_settings[i][2] == 0xFF))
        {
            break;
        }

        if ((hdmi_default_settings[i][0] == 0) && (hdmi_default_settings[i][1] == 0) && (hdmi_default_settings[i][2] == 0))
        {
            break;
        }

        val = ADV_7611_ReadByte(hdmi_default_settings[i][0] + slv_addr_offset, hdmi_default_settings[i][1]);

        if (val == hdmi_default_settings[i][2])
        {
            dlog_info("0x%x, 0x%x, 0x%x", hdmi_default_settings[i][0] + slv_addr_offset, hdmi_default_settings[i][1], val);
        }
        else
        {
            dlog_info("0x%x, 0x%x, 0x%x, Error: right value 0x%x!", hdmi_default_settings[i][0] + slv_addr_offset, hdmi_default_settings[i][1], val, hdmi_default_settings[i][2]);
        }
    }
}


uint8_t ADV_7611_IrqHandler0(void)
{
    unsigned char val = 0;    
    val = ADV_7611_ReadByte(0x98, 0x6B);
    if ((val & (ADV_7611_V_LOCKED_SET_INTERRUPT_MASK << ADV_7611_V_LOCKED_SET_INTERRUPT_POS)))        
    {
        ADV_7611_WriteByte(0x98, 0x6C, (ADV_7611_V_LOCKED_CLEAR_INTERRUPT_MASK << ADV_7611_V_LOCKED_CLEAR_INTERRUPT_POS));        
        return 1;
    }
    return 0;
 
}

uint8_t ADV_7611_IrqHandler1(void)
{
    unsigned char val = 0;        
    val = 0;
    val = ADV_7611_ReadByte((0x98+2), 0x6B);
    if ((val & (ADV_7611_V_LOCKED_SET_INTERRUPT_MASK << ADV_7611_V_LOCKED_SET_INTERRUPT_POS)))        
    {
        ADV_7611_WriteByte((0x98+2), 0x6C, (ADV_7611_V_LOCKED_CLEAR_INTERRUPT_MASK << ADV_7611_V_LOCKED_CLEAR_INTERRUPT_POS));
        return 1;
    }
    return 0; 
}


