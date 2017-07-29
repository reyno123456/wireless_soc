/*****************************************************************************
 * Copyright: 2016-2020, Artosyn. Co., Ltd.
 * File name: can.c
 * Description: mipi drive function implementation 
 * Author: Artosyn FW
 * Version: V0.01 
 * Date: 2017.01.22
 * History: 
 * 2017.01.22 the first edition.
 * *****************************************************************************/

#include "mipi.h"
#include "reg_rw.h"
#include "debuglog.h"
#include "sys_event.h"

static STRU_MIPI_INFO s_st_mipiInfo = 
{
    .u8_mipiToEncoderCh = 0,
    .u16_mipiWidth= 1280,
    .u16_mipiHight = 720,
    .u8_mipiFrameRate = 30,
};


static STRU_REG_VALUE MIPI_init_setting[] = 
{
    {0x00, 0x20}, // RX_control 
    {0x04, 0x10}, // Start size v0
    {0x08, 0x00}, // Control v0
    {0x0C, 0x04}, // Vsync counter end v0
    {0x10, 0x04}, // Vsync counter start v0
    {0x14, 0x10}, // Start size v1
    {0x18, 0x00}, // Control v1
    {0x1C, 0x04}, // Vsync counter end v1
    {0x20, 0x04}, // Vsync counter start v1
    {0x24, 0x44}, // virtual channel control
    {0x40, 0x0F}, // Frame Vsync control
    {0x80, 0x00}, // prbs test control
    {0x84, 0x00}, // clear error status [7:0]
    {0x88, 0x00}, // clear error status [15:8]
    {0x8C, 0x00}, // PRBS test counter [7:0]
    {0x90, 0x00}, // PRBS test counter [15:8]
    {0x94, 0x40}, // PRBS test counter [23:16]
    {0x98, 0x10}, // PRBS error counter
    {0x9C, 0xFF}, // Error interrupt enable [7:0]
    {0xA0, 0xFF}, // Error interrupt enable[15:8]
    {0xA4, 0x00}, // clear error status [17:16]
    {0xA8, 0x03}, // Error interrupt enable[17:16]
    {0x100, 0xC3}, // mipi_phy_ctl1
    {0x104, 0x00}, // mipi_phy_ctl2
    {0x108, 0x00}, // pd_ivref_test_mode
    {0x10C, 0x00} // phy_bist_flag
};

static int32_t MIPI_SendInfoToEncoder(void);


/**
* @brief    
* @param     
* @retval  
* @note    
*/
int32_t MIPI_Init(uint8_t u8_toEncoderCh, 
                  uint16_t u16_width,
                  uint16_t u16_hight,
                  uint8_t u8_frameRate)
{
    uint32_t u32_i;
    uint32_t *pu32_addr;
    uint32_t u32_data;
    int32_t s_result = 0;
    uint32_t u32_setLen = sizeof(MIPI_init_setting) / sizeof(MIPI_init_setting[0]);
    STRU_REG_VALUE *pst_regValue = &(MIPI_init_setting[0]);

    for(u32_i = 0; u32_i < u32_setLen; u32_i++)
    {
        Reg_Write32(MIPI_BASE_ADDR + (pst_regValue[u32_i].u32_regAddr), 
                    pst_regValue[u32_i].u32_val);
    }
    
    if ((0 == u8_toEncoderCh) || (1 == u8_toEncoderCh))
    {
    
        
        if (0 == u8_toEncoderCh)
        {
            Reg_Write32(MIPI_BASE_ADDR + 0x24, 0x44);
        }
        else if (1 == u8_toEncoderCh)
        {
           Reg_Write32(MIPI_BASE_ADDR + 0x24, 0x81);
        }
        
        s_st_mipiInfo.u8_mipiToEncoderCh = u8_toEncoderCh;
        s_st_mipiInfo.u16_mipiWidth = u16_width;
        s_st_mipiInfo.u16_mipiHight = u16_hight;
        s_st_mipiInfo.u8_mipiFrameRate = u8_frameRate;

        MIPI_SendInfoToEncoder();
    }
    else
    {
        s_result = -1;
        dlog_info("mipi to encoder channel error.");
    }

    return s_result;
}

/**
* @brief    
* @param     
* @retval  
* @note    
*/
static int32_t MIPI_SendInfoToEncoder(void)
{
    STRU_SysEvent_H264InputFormatChangeParameter p;

    p.e_h264InputSrc = ENCODER_INPUT_SRC_MIPI;
    
    p.index = s_st_mipiInfo.u8_mipiToEncoderCh;
    p.width = s_st_mipiInfo.u16_mipiWidth;
    p.hight = s_st_mipiInfo.u16_mipiHight;
    p.framerate = s_st_mipiInfo.u8_mipiFrameRate;
    SYS_EVENT_Notify(SYS_EVENT_ID_H264_INPUT_FORMAT_CHANGE, (void*)&p);

    return 0;
}


