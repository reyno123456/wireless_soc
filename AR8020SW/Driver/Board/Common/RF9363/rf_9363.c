/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: rf_9363.c
Description: The internal APIs to control the RF 9363
Author: Artosy Software Team
Version: 0.0.1
Date: 2017/07/04
History: 
        0.0.1    2016/12/20    The initial version of RF9363.c
*****************************************************************************/
#include "debuglog.h"
#include "bb_spi.h"
#include "rf_if.h"
#include "systicks.h"
#include "bb_ctrl_internal.h"



static uint8_t regout;

#define  RF9363_RF_CLOCKRATE    (9)    //1MHz clockrate

typedef struct _STRU_RF9363_REG
{
    uint8_t addr_h;
    uint8_t addr_l;
    uint8_t value;
}STRU_RF9363_REG;


static int RF9363_SPI_WriteReg_internal(uint8_t u8_data[3])
{
    int ret = 0;

    SPI_write_read(BB_SPI_BASE_IDX, u8_data, 3, 0, 0); 
    ret =  SPI_WaitIdle(BB_SPI_BASE_IDX, BB_SPI_MAX_DELAY);

    BB_SPI_init();

    return ret;
}

static int RF9363_SPI_ReadReg_internal(uint8_t u8_data[2], uint8_t *pvalue)
{
    uint8_t ret;

    STRU_SPI_InitTypes init = {
        .ctrl0   = 0x347,
        .clk_Mhz = RF9363_RF_CLOCKRATE,
        .Tx_Fthr = SPI_TXFTLR_DEF_VALUE,
        .Rx_Ftlr = SPI_RXFTLR_DEF_VALUE,
        .SER     = SPI_SSIENR_DEF_VALUE
    };

    SPI_master_init(BB_SPI_BASE_IDX, &init);
 
    SPI_write_read(BB_SPI_BASE_IDX, u8_data, 2, pvalue, 1);
    ret = SPI_WaitIdle(BB_SPI_BASE_IDX, BB_SPI_MAX_DELAY);

    BB_SPI_init();

    return ret;
}


/**
  * @brief : Write 9363 RF register by SPI 
  * @param : addr: 9363 SPI address
  * @param : data: data for 9363
  * @retval  0: sucess   1: FAIL
  */
int RF_SPI_WriteReg(uint16_t u16_addr, uint8_t u8_value)
{
    uint8_t ret;
    uint16_t addr = (u16_addr & 0x0FFF) ;
    uint8_t data[3] = { ((u16_addr>>8) | 0x80) & 0xff,  u16_addr&0xff, u8_value};

    ret = RF9363_SPI_WriteReg_internal(data);    

    return ret;
}


/**
  * @brief : Read 9363 RF register by SPI 
  * @param : addr: 9363 SPI address
  * @retval  0: sucess   1: FAIL
  */
int RF_SPI_ReadReg(uint16_t u16_addr, uint8_t *pu8_rxValue)
{
    int ret = 0;
    uint8_t data[2] = {(u16_addr>>8)&0xff,  u16_addr&0xff};

    ret = RF9363_SPI_ReadReg_internal(data, pu8_rxValue);
    return ret;
}


void config_9363_regs(STRU_RF9363_REG *RF1_9363_regs, uint16_t idx)
{
    uint8_t data[3];
    uint16_t addr;    
    int ret;

    if ( RF1_9363_regs->addr_h == 0xFF) //wait timeout
    {
       uint16_t u16_delay = ((RF1_9363_regs->addr_l << 8) | RF1_9363_regs->value);
       SysTicks_DelayMS(u16_delay);

       dlog_warning("waittimeout: 0x%x %d", u16_delay, idx);
    }
    else if ( ((RF1_9363_regs->addr_h) & 0xF0) == 0xF0) //wait regiters done
    {
        uint16_t count = 0;
        uint8_t waitbit   = (RF1_9363_regs->value & 0x0F);
        uint8_t donevalue = (RF1_9363_regs->value >> 4);
        addr = ((RF1_9363_regs->addr_h & 0x0F) << 8) | (RF1_9363_regs->addr_l);

        do
        {
            ret = RF_SPI_ReadReg(addr, data+2);
            SysTicks_DelayMS(2);
            count ++;
        }while( ( (data[2] >> waitbit) & 0x01) != donevalue && count <= 2000);

        if( count >= 2000)
        {
            dlog_error("9363: cali TIMOUT");
        }
    }
    else
    {
        uint8_t flag = (RF1_9363_regs->addr_h & 0xF0);
        if ( flag == 0x80 ) //write
        {
            if (RF1_9363_regs->addr_l == 0xA3)
            {
                //RF1_9363_regs->value |= (reg_0xa3 & 0xC0); //update bit[7:6]
                RF_SPI_WriteReg(0xa3, RF1_9363_regs->value);
                dlog_warning("write: 0xa3 %02x!!!", RF1_9363_regs->value);
            }
            else
            {
                addr = (RF1_9363_regs->addr_h <<8) | RF1_9363_regs->addr_l;
                ret = RF_SPI_WriteReg(addr, RF1_9363_regs->value);
            }
        }
        else if(flag == 0xe0)
        {/*
            addr = (RF1_9363_regs->addr_h<<8)| (RF1_9363_regs->addr_l);
            RF_SPI_ReadReg(addr, &regout);
            if( addr == 0xa3)
            {
                dlog_warning("Read 0x%x 0x%x", addr, regout);            
            }*/
        }
        else
        {
            dlog_error("reg map: %d 0x%x 0x%x 0x%x", idx, RF1_9363_regs->addr_h , RF1_9363_regs->addr_l, RF1_9363_regs->value);
        }
    }
}


void checkSramconfig(void)
{
    STRU_SettingConfigure* cfg_addr = NULL;
    GET_CONFIGURE_FROM_FLASH(cfg_addr);

    STRU_RF9363_REG *RF1_9363_regs = (STRU_RF9363_REG *)(&cfg_addr->RF9363_common_regs[0][0]);
    if( RF1_9363_regs->addr_h != 0x83 || RF1_9363_regs->addr_l != 0xdf || RF1_9363_regs->value !=0x01)
    {
        dlog_error("1:%x %x %x", RF1_9363_regs->addr_h, RF1_9363_regs->addr_l, RF1_9363_regs->value);        
    }

    if( RF1_9363_regs[2570].addr_h != 0x81 || RF1_9363_regs[2570].addr_l != 0x5c || RF1_9363_regs[2570].value !=0x62)
    {
        dlog_error("2:%x %x %x", RF1_9363_regs[2570].addr_h, RF1_9363_regs[2570].addr_l, RF1_9363_regs[2570].value);        
    }                          

    RF1_9363_regs = (STRU_RF9363_REG *)(&cfg_addr->RF9363_ground_regs[1][0]);
    if( RF1_9363_regs->addr_h != 0x80 || RF1_9363_regs->addr_l != 0x02 || RF1_9363_regs->value !=0xce)
    {
        dlog_error("3:%x %x %x", RF1_9363_regs->addr_h, RF1_9363_regs->addr_l, RF1_9363_regs->value);
    }

    if( RF1_9363_regs[51].addr_h != 0x82 || RF1_9363_regs[51].addr_l != 0x27 || RF1_9363_regs[51].value !=0x00)
    {
        dlog_error("4:%x %x %x", RF1_9363_regs[51].addr_h, RF1_9363_regs[51].addr_l, RF1_9363_regs[51].value);        
    }     

    RF1_9363_regs = (STRU_RF9363_REG *)cfg_addr->RF9363_sky_regs;
    if( RF1_9363_regs->addr_h != 0x80 || RF1_9363_regs->addr_l != 0x02 || RF1_9363_regs->value !=0xce)
    {
        dlog_error("5:%x %x %x", RF1_9363_regs->addr_h, RF1_9363_regs->addr_l, RF1_9363_regs->value);        
    }

    if( RF1_9363_regs[58].addr_h != 0x82 || RF1_9363_regs[58].addr_l != 0x27 || RF1_9363_regs[58].value !=0x00)
    {
        dlog_error("6:%x %x %x", RF1_9363_regs[51].addr_h, RF1_9363_regs[51].addr_l, RF1_9363_regs[51].value);        
    } 
}

void rf9363_checkDeviceId(void)
{
    uint8_t deviceid;

    RF_SPI_ReadReg(0x37, &deviceid);
    if(deviceid != 0x0a)
    {
        dlog_error("Error: deviceiD =0x%x", deviceid);
    }
}

/**
  * @brief : init RF9363 register
  * @param : addr: 9363 SPI address
  * @retval  None
  */
void RF_init(STRU_BoardCfg *boardCfg, ENUM_BB_MODE en_mode)
{
    uint16_t idx;
    uint16_t cnt;

    STRU_SettingConfigure* cfg_addr = NULL;
    GET_CONFIGURE_FROM_FLASH(cfg_addr);

    dlog_info("9363 init start");
    checkSramconfig();

    STRU_RF9363_REG *RF1_9363_regs = (STRU_RF9363_REG *)(&cfg_addr->RF9363_common_regs[0][0]);

    BB_WriteRegMask(PAGE2, 0x00, 0x01, 0x01);        //hold BB sw reset

    BB_SPI_curPageWriteByte(0x01, 0x01);             //bypass: SPI change into 9363
    rf9363_checkDeviceId();

    cnt = sizeof(cfg_addr->RF9363_common_regs)/sizeof(cfg_addr->RF9363_common_regs[0]);
    RF1_9363_regs = (STRU_RF9363_REG *)cfg_addr->RF9363_common_regs;
    for(idx = 0; idx < cnt-1; idx++)
    {
        config_9363_regs(RF1_9363_regs+idx, idx);
    }

    if (en_mode == BB_GRD_MODE)
    {
        cnt = sizeof(cfg_addr->RF9363_ground_regs)/sizeof(cfg_addr->RF9363_ground_regs[0]);
        RF1_9363_regs = (STRU_RF9363_REG *)(&cfg_addr->RF9363_ground_regs[1][0]);
        for(idx = 0; idx < cnt ; idx++)
        {
            config_9363_regs(RF1_9363_regs+idx, idx);
        }
    }
    else
    {
        cnt = sizeof(cfg_addr->RF9363_sky_regs)/sizeof(cfg_addr->RF9363_sky_regs[0]);
        RF1_9363_regs = (STRU_RF9363_REG *)(cfg_addr->RF9363_sky_regs);

        for(idx = 0; idx < cnt ; idx++)
        {
            config_9363_regs(RF1_9363_regs+idx, idx);
        }
    }

    RF_SPI_WriteReg(0x14, 0x09);
    RF_SPI_WriteReg(0x14, 0x29);

    BB_SPI_curPageWriteByte(0x01,0x02);             //SPI change into 8020
    BB_WriteRegMask(PAGE2, 0x00, 0x01, 0x00);       //release BB sw reset

    dlog_info("9363 init end");   
}


void BB_RF_band_switch(ENUM_RF_BAND rf_band)
{
    return;
}


void RF_CaliProcess(ENUM_BB_MODE en_mode, STRU_BoardCfg *boardCfg)
{
    return;
}


void BB_grd_notify_it_skip_freq_1(void)
{
    return;
}

void BB_grd_notify_it_skip_freq(ENUM_RF_BAND band, uint8_t u8_ch)
{

}

uint8_t BB_write_ItRegs(uint32_t u32_it)
{
    return 0;
}



uint8_t BB_set_ITfrq(ENUM_RF_BAND band, uint8_t ch)
{
    return 0;
}

uint8_t BB_write_RcRegs(uint32_t u32_rc)
{
    return 0;
}


uint8_t BB_set_Rcfrq(ENUM_RF_BAND band, uint8_t ch)
{
    return 0;
}


uint8_t BB_set_sweepfrq(ENUM_RF_BAND band, ENUM_CH_BW e_bw, uint8_t ch)
{
    return 0;
}
