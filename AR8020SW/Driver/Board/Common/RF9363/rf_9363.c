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
#include "cfg_parser.h"
#include "boardParameters.h"

#define  RF9363_RF_CLOCKRATE    (9)    //1MHz clockrate

static uint8_t *RF9363_common_regs;
static uint32_t u32_common_regscnt;

static uint8_t *RF9363_sky_regs;
static uint32_t u32_sky_regscnt;

static uint8_t *RF9363_grd_regs;
static uint32_t u32_grd_regscnt;

static STRU_FRQ_CHANNEL *pstru_rcFreq;
static uint32_t u32_rcFreq;

static STRU_FRQ_CHANNEL *pstru_sweepFreq_10M;
static uint32_t u32_sweepFreqCnt;

static STRU_FRQ_CHANNEL *pstru_itFreq;
static uint32_t u32_itFreq;


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


void config_9363_regs(STRU_RF_REG *RF1_9363_regs, uint16_t idx)
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
            }
            else
            {
                addr = (RF1_9363_regs->addr_h <<8) | RF1_9363_regs->addr_l;
                ret = RF_SPI_WriteReg(addr, RF1_9363_regs->value);
            }
        }
        else if(flag == 0xe0)
        {
            uint8_t regout;
            RF_SPI_ReadReg(addr, &regout);
        }
        else
        {
            dlog_error("reg map: %d 0x%x 0x%x 0x%x", idx, RF1_9363_regs->addr_h , RF1_9363_regs->addr_l, RF1_9363_regs->value);
        }
    }
}


void checkSramconfig(void)
{
    STRU_RF_REG *RF1_9363_regs = (STRU_RF_REG *)RF9363_common_regs;
    if( RF1_9363_regs->addr_h != 0x83 || RF1_9363_regs->addr_l != 0xdf || RF1_9363_regs->value !=0x01)
    {
        dlog_error("1:%x %x %x", RF1_9363_regs->addr_h, RF1_9363_regs->addr_l, RF1_9363_regs->value);        
    }

    if( RF1_9363_regs[2570].addr_h != 0x81 || RF1_9363_regs[2570].addr_l != 0x5c || RF1_9363_regs[2570].value !=0x62)
    {
        dlog_error("2:%x %x %x", RF1_9363_regs[2570].addr_h, RF1_9363_regs[2570].addr_l, RF1_9363_regs[2570].value);        
    }                          

    RF1_9363_regs = (STRU_RF_REG *)RF9363_grd_regs;
    if( RF1_9363_regs->addr_h != 0x80 || RF1_9363_regs->addr_l != 0x02 || RF1_9363_regs->value !=0xce)
    {
        dlog_error("3:%x %x %x", RF1_9363_regs->addr_h, RF1_9363_regs->addr_l, RF1_9363_regs->value);
    }

    if( RF1_9363_regs[51].addr_h != 0x82 || RF1_9363_regs[51].addr_l != 0x27 || RF1_9363_regs[51].value !=0x00)
    {
        dlog_error("4:%x %x %x", RF1_9363_regs[51].addr_h, RF1_9363_regs[51].addr_l, RF1_9363_regs[51].value);        
    }     

    RF1_9363_regs = (STRU_RF_REG *)RF9363_sky_regs;
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


static void RF9363_getCfgData(ENUM_BB_MODE en_mode, STRU_cfgBin *cfg)
{
    STRU_cfgNode *pcfg_node;

    pcfg_node = CFGBIN_GetNode(cfg, RF_INIT_REG_NODE_ID_0);
    if (NULL!=pcfg_node)
    {
        RF9363_common_regs = (uint8_t *)(pcfg_node + 1);
        u32_common_regscnt  = pcfg_node->nodeElemCnt;
    }

    pcfg_node = CFGBIN_GetNode(cfg, RF_SKY_INIT_REG_ID_0);
    if (NULL!=pcfg_node)
    {
        RF9363_sky_regs = (uint8_t *)(pcfg_node +1); 
        u32_sky_regscnt  = pcfg_node->nodeElemCnt;
    }

    pcfg_node = CFGBIN_GetNode(cfg, RF_GRD_INIT_REG_ID_0);
    if (NULL!=pcfg_node)
    {
        RF9363_grd_regs = (uint8_t *)(pcfg_node +1); 
        u32_grd_regscnt  = pcfg_node->nodeElemCnt;
    }

    pcfg_node  = CFGBIN_GetNode(cfg, RF9363_RC_VHF_10M_FRQ_ID);
    if (NULL!=pcfg_node)
    {
        pstru_rcFreq = (STRU_FRQ_CHANNEL *)(pcfg_node + 1);
        u32_rcFreq = pcfg_node->nodeElemCnt;
    }

    pcfg_node  = CFGBIN_GetNode(cfg, RF9363_IT_VHF_10M_FRQ_ID);
    if (NULL!=pcfg_node)
    {
        pstru_sweepFreq_10M  = (STRU_FRQ_CHANNEL *)(pcfg_node + 1);
        u32_sweepFreqCnt = pcfg_node->nodeElemCnt;

        pstru_itFreq    = pstru_sweepFreq_10M;
        u32_itFreq       = pcfg_node->nodeElemCnt;
    }
}


/**
  * @brief : init RF9363 register
  * @param : addr: 9363 SPI address
  * @retval  None
  */
void RF_init(ENUM_BB_MODE en_mode)
{
    uint16_t idx;
    uint16_t cnt;
    
    RF9363_getCfgData(en_mode, (STRU_cfgBin *)SRAM_CONFIGURE_MEMORY_ST_ADDR);
    checkSramconfig();

    STRU_RF_REG *RF1_9363_regs = (STRU_RF_REG *)RF9363_common_regs;

    BB_WriteRegMask(PAGE2, 0x00, 0x01, 0x01);        //hold BB sw reset

    BB_SPI_curPageWriteByte(0x01, 0x01);             //bypass: SPI change into 9363
    rf9363_checkDeviceId();

    cnt = u32_common_regscnt;
    RF1_9363_regs = (STRU_RF_REG *)RF9363_common_regs;
    for(idx = 0; idx < cnt-1; idx++)
    {
        config_9363_regs(RF1_9363_regs+idx, idx);
    }

    if (en_mode == BB_GRD_MODE)
    {
        cnt = u32_grd_regscnt;
        RF1_9363_regs = (STRU_RF_REG *)RF9363_grd_regs;
    }
    else
    {
        cnt = u32_sky_regscnt;
        RF1_9363_regs = (STRU_RF_REG *)RF9363_sky_regs;
    }

    for(idx = 0; idx < cnt ; idx++)
    {
        config_9363_regs(RF1_9363_regs+idx, idx);
    }

    RF_SPI_WriteReg(0x14, 0x09);
    RF_SPI_WriteReg(0x14, 0x29);

    BB_SPI_curPageWriteByte(0x01,0x02);             //SPI change into 8020
    BB_WriteRegMask(PAGE2, 0x00, 0x01, 0x00);       //release BB sw reset
}


void BB_RF_band_switch(ENUM_RF_BAND rf_band)
{
    return;
}


void RF_CaliProcess(ENUM_BB_MODE en_mode)
{
    return;
}


void BB_grd_NotifyItFreqByCh(ENUM_RF_BAND band, uint8_t u8_ch)
{
    STRU_FRQ_CHANNEL *pstru_frq = pstru_itFreq;

    BB_WriteReg(PAGE2, IT_FRQ_0, pstru_frq[u8_ch].frq1);
    BB_WriteReg(PAGE2, IT_FRQ_1, pstru_frq[u8_ch].frq2);
    BB_WriteReg(PAGE2, IT_FRQ_2, pstru_frq[u8_ch].frq3);
    BB_WriteReg(PAGE2, IT_FRQ_3, pstru_frq[u8_ch].frq4);
    BB_WriteReg(PAGE2, IT_FRQ_4, pstru_frq[u8_ch].frq5);
}


void BB_grd_NotifyItFreqByValue(uint32_t u32_itFrq)
{
    BB_WriteReg(PAGE2, IT_FRQ_0, context.stru_itRegs.frq1);
    BB_WriteReg(PAGE2, IT_FRQ_1, context.stru_itRegs.frq2);
    BB_WriteReg(PAGE2, IT_FRQ_2, context.stru_itRegs.frq3);
    BB_WriteReg(PAGE2, IT_FRQ_3, context.stru_itRegs.frq4);
    BB_WriteReg(PAGE2, IT_FRQ_4, context.stru_itRegs.frq5);

}

uint8_t BB_write_ItRegsByArr(uint8_t *pu8_it)
{
    context.stru_itRegs.frq1 = pu8_it[0];
    context.stru_itRegs.frq2 = pu8_it[1];
    context.stru_itRegs.frq3 = pu8_it[2];
    context.stru_itRegs.frq4 = pu8_it[3];
    context.stru_itRegs.frq5 = pu8_it[4];

    BB_WriteReg(PAGE2, 0x10, context.stru_itRegs.frq1);
    BB_WriteReg(PAGE2, 0x11, context.stru_itRegs.frq2);
    BB_WriteReg(PAGE2, 0x12, context.stru_itRegs.frq3);
    BB_WriteReg(PAGE2, 0x13, context.stru_itRegs.frq4);
    BB_WriteReg(PAGE2, 0x14, context.stru_itRegs.frq5);
    
    return 0;
}

void BB_write_ItRegs(uint32_t u32_it)
{

}


uint8_t BB_set_ItFrqByCh(ENUM_RF_BAND band, uint8_t ch)
{
    STRU_FRQ_CHANNEL *it_ch_ptr = pstru_itFreq;

    context.stru_itRegs.frq1 = it_ch_ptr[ch].frq1;
    context.stru_itRegs.frq2 = it_ch_ptr[ch].frq2;
    context.stru_itRegs.frq3 = it_ch_ptr[ch].frq3;
    context.stru_itRegs.frq4 = it_ch_ptr[ch].frq4;
    context.stru_itRegs.frq5 = it_ch_ptr[ch].frq5;

    BB_WriteReg(PAGE2, 0x10, it_ch_ptr[ch].frq1);
    BB_WriteReg(PAGE2, 0x11, it_ch_ptr[ch].frq2);
    BB_WriteReg(PAGE2, 0x12, it_ch_ptr[ch].frq3);
    BB_WriteReg(PAGE2, 0x13, it_ch_ptr[ch].frq4);
    BB_WriteReg(PAGE2, 0x14, it_ch_ptr[ch].frq5);
    return 0;
}

uint8_t BB_write_RcRegs(uint32_t u32_rc)
{
    return 0;
}


uint8_t BB_set_Rcfrq(ENUM_RF_BAND band, uint8_t ch)
{
    STRU_FRQ_CHANNEL *pu8_rcRegs = pstru_rcFreq;

    context.stru_rcRegs.frq1 = pu8_rcRegs[ch].frq1;
    context.stru_rcRegs.frq2 = pu8_rcRegs[ch].frq2;
    context.stru_rcRegs.frq3 = pu8_rcRegs[ch].frq3;
    context.stru_rcRegs.frq4 = pu8_rcRegs[ch].frq4;
    context.stru_rcRegs.frq5 = pu8_rcRegs[ch].frq5;

    BB_WriteReg(PAGE2, 0x1A, pu8_rcRegs[ch].frq1);
    BB_WriteReg(PAGE2, 0x1B, pu8_rcRegs[ch].frq2);
    BB_WriteReg(PAGE2, 0x1C, pu8_rcRegs[ch].frq3);
    BB_WriteReg(PAGE2, 0x1D, pu8_rcRegs[ch].frq4); 
    BB_WriteReg(PAGE2, 0x1E, pu8_rcRegs[ch].frq5); 
    
    return 0;
}


uint8_t BB_set_SweepFrq(ENUM_RF_BAND band, ENUM_CH_BW e_bw, uint8_t ch)
{
    STRU_FRQ_CHANNEL *ch_ptr = pstru_sweepFreq_10M;

    BB_WriteReg(PAGE2, 0x15, ch_ptr[ch].frq1);
    BB_WriteReg(PAGE2, 0x16, ch_ptr[ch].frq2);
    BB_WriteReg(PAGE2, 0x17, ch_ptr[ch].frq3);
    BB_WriteReg(PAGE2, 0x18, ch_ptr[ch].frq4);
    BB_WriteReg(PAGE2, 0x19, ch_ptr[ch].frq5);
    return 0;
}
