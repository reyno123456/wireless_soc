/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: RF_8003.c
Description: The internal APIs to control the RF 8003
Author: Artosy Software Team
Version: 0.0.1
Date: 2016/12/20
History: 
        0.0.1    2016/12/20    The initial version of rf8003.c
*****************************************************************************/
#include "debuglog.h"
#include "bb_spi.h"
#include "rf_if.h"
#include "systicks.h"
#include "bb_ctrl_internal.h"
#include "cfg_parser.h"

#define     AAGC_GAIN_FAR_8003                   (0x12)
#define     AAGC_GAIN_NEAR_8003                  (0x3F)


static uint8_t  u8_caliRegArray[10];

static uint8_t *pu8_rf0_regs;
static uint8_t *pu8_rf1_regs;

static STRU_FRQ_CHANNEL *pstru_rcFreq_2g;
static STRU_FRQ_CHANNEL *pstru_sweepFreq_2g_10m;
static STRU_FRQ_CHANNEL *pstru_sweepFreq_2g_20m;
static STRU_FRQ_CHANNEL *pstru_itFreq_2g;

static uint8_t u8_rcFreqCnt_2g;
static uint8_t u8_itFreqCnt_2g;

static STRU_RF_REG *pstru_rf0_regBeforeCali;
static STRU_RF_REG *pstru_rf1_regBeforeCali;

static STRU_RF_REG *pstru_rf0_regAfterCali;
static STRU_RF_REG *pstru_rf1_regAfterCali;

static STRU_BOARD_RF_PARA *pstru_rf_boardcfg;


void RF8003_getCfgData(ENUM_BB_MODE en_mode, STRU_cfgBin *cfg)
{
    STRU_cfgNode  *p_bbrf_boardNode;
    STRU_cfgNode  *p_bbrf_dataNode;

    STRU_cfgNode  *rfcfg_node;

    p_bbrf_boardNode = CFGBIN_GetNode(cfg, RF_INIT_REG_NODE_ID_0);
    if (NULL != p_bbrf_boardNode)
    {
        pu8_rf0_regs = (uint8_t *)(p_bbrf_boardNode + 1);
    }

    p_bbrf_boardNode = CFGBIN_GetNode(cfg, RF_INIT_REG_NODE_ID_1);
    if (NULL != p_bbrf_boardNode)
    {
        pu8_rf1_regs = (uint8_t *)(p_bbrf_boardNode + 1);
    }

    p_bbrf_boardNode  = CFGBIN_GetNode(cfg, RF_BOARDCFG_PARA_ID);
    if (NULL!=p_bbrf_boardNode)
    {
        pstru_rf_boardcfg = (STRU_BOARD_RF_PARA *)(p_bbrf_boardNode + 1);
        DLOG_Info("board name:%s", pstru_rf_boardcfg->boardCfgName);
    }

    p_bbrf_dataNode  = CFGBIN_GetNode(cfg, RF_BOARDCFG_DATA_ID);
    if (NULL != p_bbrf_dataNode)
    {
        if (en_mode == BB_SKY_MODE)
        {
            pstru_rf0_regBeforeCali = (STRU_RF_REG *)(p_bbrf_dataNode + 1);
            pstru_rf1_regBeforeCali = pstru_rf0_regBeforeCali; 

            pstru_rf0_regAfterCali = pstru_rf0_regBeforeCali + pstru_rf_boardcfg->u8_rf0SkyRegsCnt + pstru_rf_boardcfg->u8_rf0GrdRegsCnt;
            pstru_rf1_regAfterCali = pstru_rf0_regAfterCali;
        }
        else
        {
            pstru_rf0_regBeforeCali =  (STRU_RF_REG *)(p_bbrf_dataNode + 1) + pstru_rf_boardcfg->u8_rf0SkyRegsCnt;
            pstru_rf1_regBeforeCali =  pstru_rf0_regBeforeCali + pstru_rf_boardcfg->u8_rf0GrdRegsCnt;
        
            pstru_rf0_regAfterCali = pstru_rf1_regBeforeCali + pstru_rf_boardcfg->u8_rf0SkyRegsCntAfterCali;
            pstru_rf1_regAfterCali = pstru_rf0_regAfterCali  + pstru_rf_boardcfg->u8_rf0GrdRegsCntAfterCali;
        }
    }

    //2g
    rfcfg_node = CFGBIN_GetNode(cfg, RF_RC_BAND0_10M_FRQ_ID);
    if (NULL != rfcfg_node)
    {
        pstru_rcFreq_2g = (STRU_FRQ_CHANNEL *)(rfcfg_node + 1);
        u8_rcFreqCnt_2g = rfcfg_node->nodeElemCnt;
    }

    rfcfg_node= CFGBIN_GetNode(cfg, RF_IT_BAND0_10M_FRQ_ID);
    if (NULL != rfcfg_node)
    {
        pstru_sweepFreq_2g_10m = (STRU_FRQ_CHANNEL *)(rfcfg_node + 1);
        pstru_itFreq_2g = pstru_sweepFreq_2g_10m;
        u8_itFreqCnt_2g = rfcfg_node->nodeElemCnt;
    }

    rfcfg_node= CFGBIN_GetNode(cfg, RF_SWEEP_BAND0_20M_FRQ_ID);
    if (NULL != rfcfg_node)
    {
        pstru_sweepFreq_2g_20m = (STRU_FRQ_CHANNEL *)(rfcfg_node +1);
    }
}


#define  RF8003_RF_CLOCKRATE    (1)    //1MHz clockrate

static int RF8003_SPI_WriteReg_internal(uint8_t u8_addr, uint8_t u8_data)
{
    int ret = 0;
    uint8_t wdata[] = {0x80, (u8_addr <<1), u8_data};   //RF_8003S_SPI: wr: 0x80 ;

    //SPI_master_init(BB_SPI_BASE_IDX, &init);
    SPI_write_read(BB_SPI_BASE_IDX, wdata, sizeof(wdata), 0, 0); 
    ret =  SPI_WaitIdle(BB_SPI_BASE_IDX, BB_SPI_MAX_DELAY);

    return ret;
}


static int RF8003_SPI_ReadReg_internal(uint8_t u8_addr)
{
    uint8_t wdata[2] = {0x00, (u8_addr<<1)};      //RF_8003_SPI:  rd: 0x00
    uint8_t rdata;
    
    //use low speed for the RF8003 read, from test, read fail if use the same clockrate as baseband
    STRU_SPI_InitTypes init = {
        .ctrl0   = SPI_CTRL0_DEF_VALUE,
        .clk_Mhz = RF8003_RF_CLOCKRATE,
        .Tx_Fthr = SPI_TXFTLR_DEF_VALUE,
        .Rx_Ftlr = SPI_RXFTLR_DEF_VALUE,
        .SER     = SPI_SSIENR_DEF_VALUE
    };

    SPI_master_init(BB_SPI_BASE_IDX, &init);
 
    SPI_write_read(BB_SPI_BASE_IDX, wdata, sizeof(wdata), &rdata, 1);
    SPI_WaitIdle(BB_SPI_BASE_IDX, BB_SPI_MAX_DELAY);

    BB_SPI_init();

    return rdata;
}

/**
  * @brief : Write 8003 RF register by SPI 
  * @param : addr: 8003 SPI address
  * @param : data: data for 8003
  * @retval  0: sucess   1: FAIL
  */
int RF_SPI_WriteReg(uint16_t u8_addr, uint8_t u8_data)
{
    return RF8003_SPI_WriteReg_internal((uint8_t)u8_addr, u8_data);
}

/**
  * @brief : Read 8003 RF register by SPI 
  * @param : addr: 8003 SPI address
  * @retval  0: sucess   1: FAIL
  */
int RF_SPI_ReadReg(uint16_t u8_addr, uint8_t *pu8_rxValue)
{
    uint8_t tmp = RF8003_SPI_ReadReg_internal( (uint8_t)u8_addr);
    if( pu8_rxValue)
    {
        *pu8_rxValue = tmp;
    }
    else
    {
        dlog_error("pu8_rxValue == NULL");
    }

    return 0;
}

/**
  * @brief : init RF8003 register
  * @param : addr: 8003 SPI address
  * @retval  None
  */
void RF_init(ENUM_BB_MODE en_mode)
{
    uint8_t idx;
    uint8_t cnt;
    uint8_t num;

    STRU_RF_REG * pstru_rfReg = NULL;

    RF8003_getCfgData(en_mode, (STRU_cfgBin *)SRAM_CONFIGURE_MEMORY_ST_ADDR);

    //RF 0
    {
        BB_SPI_curPageWriteByte(0x01,0x01);             //bypass: SPI change into 8003

        if ( pstru_rf_boardcfg != NULL )
        {
            if ( en_mode == BB_SKY_MODE )               //sky mode register replace
            {
                num = pstru_rf_boardcfg->u8_rf0SkyRegsCnt;
                pstru_rfReg = pstru_rf0_regBeforeCali;
            }
            else                                        //ground mode register replace
            {
                num = pstru_rf_boardcfg->u8_rf0GrdRegsCnt;
                pstru_rfReg = (STRU_RF_REG * )pstru_rf0_regBeforeCali;
            }

            for (cnt = 0; (pstru_rfReg != NULL) && (cnt < num); cnt++)
            {
                pu8_rf0_regs[pstru_rfReg[cnt].addr_l] = pstru_rfReg[cnt].value;
            } 
        }

        for(idx = 0; idx < 64; idx++)
        {
            RF8003_SPI_WriteReg_internal(idx, pu8_rf0_regs[idx]);
        }

        {
            //add patch, reset 8003
            RF8003_SPI_WriteReg_internal(0x15, 0x51);
            RF8003_SPI_WriteReg_internal(0x15, 0x50);
        }
        
        BB_SPI_curPageWriteByte(0x01,0x02);             //SPI change into 8020
    }    
}


static void RF8003_afterCali(ENUM_BB_MODE en_mode, STRU_BOARD_RF_PARA *pstru_rf_boardcfg)
{
    STRU_RF_REG * pu8_rf1_regs, * rf2_regs;
    uint8_t cnt;
    uint8_t rf_regcnt1, rf_regcnt2;

    if( NULL == pstru_rf_boardcfg)
    {
        return;
    }

    if (en_mode == BB_SKY_MODE)
    {
        rf_regcnt1 = pstru_rf_boardcfg->u8_rf0SkyRegsCntAfterCali;
        rf_regcnt2 = 0;

        pu8_rf1_regs   = pstru_rf0_regAfterCali;
        rf2_regs   = NULL;
    }
    else
    {
        rf_regcnt1 = pstru_rf_boardcfg->u8_rf0GrdRegsCntAfterCali;
        rf_regcnt2 = pstru_rf_boardcfg->u8_rf0GrdRegsCntAfterCali;

        pu8_rf1_regs   = (STRU_RF_REG * )pstru_rf0_regAfterCali;
        rf2_regs   = (STRU_RF_REG * )pstru_rf1_regAfterCali;
    }

    if ( rf_regcnt1 > 0 && pu8_rf1_regs != NULL)
    {
        BB_SPI_curPageWriteByte(0x01,0x01);             //bypass: SPI change into 1st 8003
        
        for(cnt = 0; cnt < rf_regcnt1; cnt++)
        {
            RF8003_SPI_WriteReg_internal( pu8_rf1_regs[cnt].addr_l, pu8_rf1_regs[cnt].value);
        }

        {
            //add patch, reset 8003
            uint16_t delay = 0;
            RF8003_SPI_WriteReg_internal(0x15, 0x51);
            while(delay ++ < 1000);
            RF8003_SPI_WriteReg_internal(0x15, 0x50);
        } 
        
        BB_SPI_curPageWriteByte(0x01,0x02);             //SPI change into 8020    
    }

    if (pstru_rf_boardcfg->u8_rfCnt > 1 && rf_regcnt2 > 0 && rf2_regs != NULL)
    {
        BB_SPI_curPageWriteByte(0x01,0x03);             //bypass: SPI change into 2rd 8003
        
        for(cnt = 0; cnt < rf_regcnt2; cnt++)
        {
            RF8003_SPI_WriteReg_internal( rf2_regs[cnt].addr_l, rf2_regs[cnt].value);
        }

        {
            //add patch, reset 8003
            uint16_t delay = 0;
            RF8003_SPI_WriteReg_internal(0x15, 0x51);
            while(delay ++ < 1000);            
            RF8003_SPI_WriteReg_internal(0x15, 0x50);
        } 
        
        BB_SPI_curPageWriteByte(0x01,0x02);             //SPI change into 8020    
    }

    BB_SPI_init();
}



static int BB_before_RF_cali(void)
{
    BB_WriteRegMask(PAGE0, 0x20, 0x00, 0x0c);
}


void read_cali_register(uint8_t *buf)
{
    buf[0] = BB_ReadReg(PAGE0, 0xd0);
    buf[1] = BB_ReadReg(PAGE0, 0xd1);
    buf[2] = BB_ReadReg(PAGE0, 0xd2);
    buf[3] = BB_ReadReg(PAGE0, 0xd3);
    buf[4] = BB_ReadReg(PAGE0, 0xd4);
    buf[5] = BB_ReadReg(PAGE0, 0xd5);
    buf[6] = BB_ReadReg(PAGE0, 0xd6);
    buf[7] = BB_ReadReg(PAGE0, 0xd7);
    buf[8] = BB_ReadReg(PAGE0, 0xd8);
    buf[9] = BB_ReadReg(PAGE0, 0xd9);
}


static void BB_RF_start_cali( void )
{
    uint8_t data;

    //step 1
    //1.1 Enable RF calibration 0x61= 0x0F
    BB_WriteReg(PAGE0, TX_CALI_ENABLE, 0x0F);

    //1.2: soft reset0
    //page0 0x64[4] = 1'b1
    data = BB_ReadReg(PAGE0, 0x64);
    data = data | 0x10;
    BB_WriteReg(PAGE0, 0x64, data);
    //page0 0x64[4] = 1'b0
    data = data & 0xEF;
    BB_WriteReg(PAGE0, 0x64, data);

    data = BB_ReadReg(PAGE0, 0x00);
    data |= 0x01;
    BB_WriteReg(PAGE0, 0x00, data);
    //page0 0x64[4] = 1'b0
    data &= 0xFE;
    BB_WriteReg(PAGE0, 0x00, data);

    //1.3: wait 1s
    SysTicks_DelayMS(1000);

    //select the 2G,  Read RF calibration register values
    data = BB_ReadReg(PAGE0, 0x64);
    BB_WriteReg(PAGE0, 0x64, (data&0x7F));
    read_cali_register(u8_caliRegArray);
}



void BB_RF_band_switch(ENUM_RF_BAND rf_band)
{

}


void RF_CaliProcess(ENUM_BB_MODE en_mode)
{
    BB_before_RF_cali();

    BB_RF_start_cali();

    BB_WriteReg(PAGE0, TX_CALI_ENABLE, 0x00);   //disable calibration

    RF8003_afterCali(en_mode, pstru_rf_boardcfg);
}


void BB_grd_NotifyItFreqByValue(uint32_t u32_itFrq)
{
    BB_WriteReg(PAGE2, IT_FRQ_0, ((u32_itFrq >> 24)&0xff));
    BB_WriteReg(PAGE2, IT_FRQ_1, ((u32_itFrq >> 16)&0xff));
    BB_WriteReg(PAGE2, IT_FRQ_2, ((u32_itFrq >>  8)&0xff));
    BB_WriteReg(PAGE2, IT_FRQ_3, ( u32_itFrq & 0xff));
}


void BB_grd_NotifyItFreqByCh(ENUM_RF_BAND band, uint8_t u8_ch)
{
    BB_WriteReg(PAGE2, IT_FRQ_0, pstru_itFreq_2g[u8_ch].frq1);
    BB_WriteReg(PAGE2, IT_FRQ_1, pstru_itFreq_2g[u8_ch].frq2);
    BB_WriteReg(PAGE2, IT_FRQ_2, pstru_itFreq_2g[u8_ch].frq3);
    BB_WriteReg(PAGE2, IT_FRQ_3, pstru_itFreq_2g[u8_ch].frq4);
}


void BB_write_ItRegs(uint32_t u32_itFrq)
{
    context.stru_itRegs.frq1 = (uint8_t)(u32_itFrq >> 24) & 0xff;
    context.stru_itRegs.frq2 = (uint8_t)(u32_itFrq >> 16) & 0xff;
    context.stru_itRegs.frq3 = (uint8_t)(u32_itFrq >>  8) & 0xff;
    context.stru_itRegs.frq4 = (uint8_t)(u32_itFrq) & 0xff;

    BB_WriteReg(PAGE2, AGC3_0, context.stru_itRegs.frq1);
    BB_WriteReg(PAGE2, AGC3_1, context.stru_itRegs.frq2);
    BB_WriteReg(PAGE2, AGC3_2, context.stru_itRegs.frq3);
    BB_WriteReg(PAGE2, AGC3_3, context.stru_itRegs.frq4);
}

uint8_t BB_write_ItRegsByArr(uint8_t *pu8_it)
{  
    return 0;
}

uint8_t BB_set_ItFrqByCh(ENUM_RF_BAND band, uint8_t ch)
{
    context.stru_itRegs.frq1 = pstru_itFreq_2g[ch].frq1;
    context.stru_itRegs.frq2 = pstru_itFreq_2g[ch].frq2;
    context.stru_itRegs.frq3 = pstru_itFreq_2g[ch].frq3;
    context.stru_itRegs.frq4 = pstru_itFreq_2g[ch].frq4;

    BB_WriteReg(PAGE2, AGC3_0, pstru_itFreq_2g[ch].frq1);
    BB_WriteReg(PAGE2, AGC3_1, pstru_itFreq_2g[ch].frq2);
    BB_WriteReg(PAGE2, AGC3_2, pstru_itFreq_2g[ch].frq3);
    BB_WriteReg(PAGE2, AGC3_3, pstru_itFreq_2g[ch].frq4);
}


uint8_t BB_write_RcRegs(uint32_t u32_rc)
{
    context.stru_rcRegs.frq1 = (uint8_t)(u32_rc >> 24) & 0xff;
    context.stru_rcRegs.frq2 = (uint8_t)(u32_rc >> 16) & 0xff;
    context.stru_rcRegs.frq3 = (uint8_t)(u32_rc >>  8) & 0xff;
    context.stru_rcRegs.frq4 = (uint8_t)(u32_rc) & 0xff;

    BB_WriteReg(PAGE2, AGC3_a, context.stru_rcRegs.frq1);
    BB_WriteReg(PAGE2, AGC3_b, context.stru_rcRegs.frq2);
    BB_WriteReg(PAGE2, AGC3_c, context.stru_rcRegs.frq3);
    BB_WriteReg(PAGE2, AGC3_d, context.stru_rcRegs.frq4);
}


uint8_t BB_set_Rcfrq(ENUM_RF_BAND band, uint8_t ch)
{
    context.stru_rcRegs.frq1 = pstru_rcFreq_2g[ch].frq1;
    context.stru_rcRegs.frq2 = pstru_rcFreq_2g[ch].frq2;
    context.stru_rcRegs.frq3 = pstru_rcFreq_2g[ch].frq3;
    context.stru_rcRegs.frq4 = pstru_rcFreq_2g[ch].frq4;

    BB_WriteReg(PAGE2, AGC3_a, pstru_rcFreq_2g[ch].frq1);
    BB_WriteReg(PAGE2, AGC3_b, pstru_rcFreq_2g[ch].frq2);
    BB_WriteReg(PAGE2, AGC3_c, pstru_rcFreq_2g[ch].frq3);
    BB_WriteReg(PAGE2, AGC3_d, pstru_rcFreq_2g[ch].frq4); 
}



uint8_t BB_set_SweepFrq(ENUM_RF_BAND band, ENUM_CH_BW e_bw, uint8_t ch)
{
    STRU_FRQ_CHANNEL *ch_ptr = ((BW_10M == e_bw) ? pstru_sweepFreq_2g_10m : pstru_sweepFreq_2g_20m);

    //set sweep frequency
    if ( band == RF_2G )
    {
        BB_WriteRegMask(PAGE2, 0x20, 0x00, 0x04);
    }
    else
    {
        // P2 0x20 [2]=1,sweep frequency,5G
        BB_WriteRegMask(PAGE2, 0x20, 0x04, 0x04);   
    }

    BB_WriteReg(PAGE2, SWEEP_FREQ_0, ch_ptr[ch].frq1);
    BB_WriteReg(PAGE2, SWEEP_FREQ_1, ch_ptr[ch].frq2);
    BB_WriteReg(PAGE2, SWEEP_FREQ_2, ch_ptr[ch].frq3);
    BB_WriteReg(PAGE2, SWEEP_FREQ_3, ch_ptr[ch].frq4);
}


uint8_t BB_GetRcFrqNum(ENUM_RF_BAND e_rfBand)
{
    return u8_rcFreqCnt_2g;
}


uint8_t BB_GetItFrqNum(ENUM_RF_BAND e_rfBand)
{
    return u8_itFreqCnt_2g;
}

uint8_t BB_SetAgcGain(uint8_t gain)
{
    if(AAGC_GAIN_FAR == gain)
    {
        BB_WriteReg(PAGE0, AGC_2, AAGC_GAIN_FAR_8003);
        BB_WriteReg(PAGE0, AGC_3, AAGC_GAIN_FAR_8003);
    }
    else if(AAGC_GAIN_NEAR == gain)
    {
        BB_WriteReg(PAGE0, AGC_2, AAGC_GAIN_NEAR_8003);
        BB_WriteReg(PAGE0, AGC_3, AAGC_GAIN_NEAR_8003);
    }
    else
    {
        ;
    }

    return 0;
}


int32_t BB_SweepEnergyCompensation(int32_t data)
{
    return ( data += ( (data > 30) ? (-123) : (-125) ) );
}

