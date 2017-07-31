/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: RF_8003s.c
Description: The internal APIs to control the RF 8003s
Author: Artosy Software Team
Version: 0.0.1
Date: 2016/12/20
History: 
        0.0.1    2016/12/20    The initial version of rf8003s.c
*****************************************************************************/
#include "debuglog.h"
#include "bb_spi.h"
#include "rf_if.h"
#include "systicks.h"
#include "bb_ctrl_internal.h"


static uint8_t cali_reg[2][10];

STRU_FRQ_CHANNEL *Rc_2G_frq    = NULL;
STRU_FRQ_CHANNEL *Sweep_2G_10m_frq = NULL;
STRU_FRQ_CHANNEL *Sweep_2G_20m_frq = NULL;
STRU_FRQ_CHANNEL *It_2G_frq    = NULL;

STRU_FRQ_CHANNEL *Rc_5G_frq = NULL;
STRU_FRQ_CHANNEL *Sweep_5G_10m_frq = NULL;
STRU_FRQ_CHANNEL *Sweep_5G_20m_frq = NULL;
STRU_FRQ_CHANNEL *It_5G_frq = NULL;



#define  RF8003S_RF_CLOCKRATE    (1)    //1MHz clockrate

static int RF8003s_SPI_WriteReg_internal(uint8_t u8_addr, uint8_t u8_data)
{
    int ret = 0;
    uint8_t wdata[] = {0x80, (u8_addr <<1), u8_data};   //RF_8003S_SPI: wr: 0x80 ; 

    //SPI_master_init(BB_SPI_BASE_IDX, &init);
    SPI_write_read(BB_SPI_BASE_IDX, wdata, sizeof(wdata), 0, 0); 
    ret =  SPI_WaitIdle(BB_SPI_BASE_IDX, BB_SPI_MAX_DELAY);

    return ret;
}


static int RF8003s_SPI_ReadReg_internal(uint8_t u8_addr)
{
    uint8_t wdata[2] = {0x00, (u8_addr<<1)};      //RF_8003S_SPI:  rd: 0x00
    uint8_t rdata;
    
    //use low speed for the RF8003 read, from test, read fail if use the same clockrate as baseband
    STRU_SPI_InitTypes init = {
        .ctrl0   = SPI_CTRL0_DEF_VALUE,
        .clk_Mhz = RF8003S_RF_CLOCKRATE,
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
    return RF8003s_SPI_WriteReg_internal((uint8_t)u8_addr, u8_data);
}

/**
  * @brief : Read 8003 RF register by SPI 
  * @param : addr: 8003 SPI address
  * @retval  0: sucess   1: FAIL
  */
int RF_SPI_ReadReg(uint16_t u8_addr, uint8_t *pu8_rxValue)
{
    uint8_t tmp = RF8003s_SPI_ReadReg_internal( (uint8_t)u8_addr);
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
  * @brief : init RF8003s register
  * @param : addr: 8003 SPI address
  * @retval  None
  */
void RF_init(STRU_BoardCfg *boardCfg, ENUM_BB_MODE en_mode)
{
    uint8_t idx;
    uint8_t cnt;
    uint8_t num;

    STRU_SettingConfigure* cfg_addr = NULL;
    GET_CONFIGURE_FROM_FLASH(cfg_addr);

    Rc_2G_frq    = (STRU_FRQ_CHANNEL *)(cfg_addr->RC_2_4G_frq);
    Sweep_2G_10m_frq = (STRU_FRQ_CHANNEL *)(cfg_addr->IT_2_4G_frq);
    Sweep_2G_20m_frq = (STRU_FRQ_CHANNEL *)(cfg_addr->IT_2_4G_20M_sweep_frq);
    It_2G_frq    = (STRU_FRQ_CHANNEL *)(cfg_addr->IT_2_4G_frq);

    Rc_5G_frq = (STRU_FRQ_CHANNEL *)(cfg_addr->RC_5G_frq);
    Sweep_5G_10m_frq = (STRU_FRQ_CHANNEL *)(cfg_addr->IT_5G_frq);
    Sweep_5G_20m_frq = (STRU_FRQ_CHANNEL *)(cfg_addr->IT_5G_20M_sweep_frq);
    It_5G_frq = (STRU_FRQ_CHANNEL *)(cfg_addr->IT_5G_frq);

    STRU_RF_REG * pstru_rfReg = NULL;
    uint8_t *RF1_8003s_regs = &(cfg_addr->rf1_configure[0]);
    uint8_t *RF2_8003s_regs = &(cfg_addr->rf2_configure[0]);

    //RF 1
    {
        BB_SPI_curPageWriteByte(0x01,0x01);             //bypass: SPI change into 8003

        if ( boardCfg != NULL )
        {        
            if ( en_mode == BB_SKY_MODE )               //sky mode register replace
            {
                num = boardCfg->u8_rf1SkyRegsCnt;
                pstru_rfReg = (STRU_RF_REG * )boardCfg->pstru_rf1SkyRegs;
            }
            else                                        //ground mode register replace
            {
                num = boardCfg->u8_rf1GrdRegsCnt;
                pstru_rfReg = (STRU_RF_REG * )boardCfg->pstru_rf1GrdRegs;
            }

            for (cnt = 0; (pstru_rfReg != NULL) && (cnt < num); cnt++)
            {
                RF1_8003s_regs[pstru_rfReg[cnt].addr] = pstru_rfReg[cnt].value;
            } 
        }
        
        for(idx = 0; idx < 128; idx++)
        {
            RF8003s_SPI_WriteReg_internal( idx, RF1_8003s_regs[idx]);
        }

        {
            //add patch, reset 8003
            RF8003s_SPI_WriteReg_internal(0x15, 0x51);
            RF8003s_SPI_WriteReg_internal(0x15, 0x50);
        }
        
        BB_SPI_curPageWriteByte(0x01,0x02);             //SPI change into 8020
    }

    //RF 2 only used in ground   
    if (boardCfg != NULL && boardCfg->u8_rfCnt > 1 && en_mode == BB_GRD_MODE )
    {
        num = boardCfg->u8_rf2GrdRegsCnt;
        pstru_rfReg = (STRU_RF_REG * )boardCfg->pstru_rf2GrdRegs;
        
        BB_SPI_curPageWriteByte(0x01,0x03);             //bypass: SPI change into 2rd 8003s
        
        for (cnt = 0; (pstru_rfReg != NULL) && (cnt < num); cnt++)
        {
            RF2_8003s_regs[pstru_rfReg[cnt].addr] = pstru_rfReg[cnt].value;
        }

        for(idx = 0; idx < 128; idx++)
        {
            RF8003s_SPI_WriteReg_internal( idx, RF2_8003s_regs[idx]);
        }

        {
            //add patch, reset 8003
            RF8003s_SPI_WriteReg_internal(0x15, 0x51);
            RF8003s_SPI_WriteReg_internal(0x15, 0x50);
        } 
        
        BB_SPI_curPageWriteByte(0x01,0x02);             //SPI change into 8020
    }
}


static void RF8003s_afterCali(ENUM_BB_MODE en_mode, STRU_BoardCfg *boardCfg)
{
    STRU_RF_REG * rf1_regs, * rf2_regs;
    uint8_t cnt;
    uint8_t rf_regcnt1, rf_regcnt2;

    if( NULL == boardCfg)
    {
        return;
    }

    if (en_mode == BB_SKY_MODE)
    {
        rf_regcnt1 = boardCfg->u8_rf1SkyRegsCntAfterCali;
        rf_regcnt2 = 0;

        rf1_regs   = (STRU_RF_REG * )boardCfg->pstru_rf1SkyRegsAfterCali;
        rf2_regs   = NULL;
    }
    else
    {
        rf_regcnt1 = boardCfg->u8_rf1GrdRegsCntAfterCali;
        rf_regcnt2 = boardCfg->u8_rf2GrdRegsCntAfterCali;

        rf1_regs   = (STRU_RF_REG * )boardCfg->pstru_rf1GrdRegsAfterCali;
        rf2_regs   = (STRU_RF_REG * )boardCfg->pstru_rf2GrdRegsAfterCali;
    }

    if ( rf_regcnt1 > 0 && rf1_regs != NULL)
    {
        BB_SPI_curPageWriteByte(0x01,0x01);             //bypass: SPI change into 1st 8003s
        
        for(cnt = 0; cnt < rf_regcnt1; cnt++)
        {
            RF8003s_SPI_WriteReg_internal( rf1_regs[cnt].addr, rf1_regs[cnt].value);
        }

        {
            //add patch, reset 8003
            uint16_t delay = 0;
            RF8003s_SPI_WriteReg_internal(0x15, 0x51);
            while(delay ++ < 1000);
            RF8003s_SPI_WriteReg_internal(0x15, 0x50);
        } 
        
        BB_SPI_curPageWriteByte(0x01,0x02);             //SPI change into 8020    
    }

    if (boardCfg->u8_rfCnt > 1 && rf_regcnt2 > 0 && rf2_regs != NULL)
    {
        BB_SPI_curPageWriteByte(0x01,0x03);             //bypass: SPI change into 2rd 8003s
        
        for(cnt = 0; cnt < rf_regcnt2; cnt++)
        {
            dlog_info("%x %x \n", rf2_regs[cnt].addr, rf2_regs[cnt].value);
            RF8003s_SPI_WriteReg_internal( rf2_regs[cnt].addr, rf2_regs[cnt].value);
        }

        {
            //add patch, reset 8003
            uint16_t delay = 0;
            RF8003s_SPI_WriteReg_internal(0x15, 0x51);
            while(delay ++ < 1000);            
            RF8003s_SPI_WriteReg_internal(0x15, 0x50);
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
    read_cali_register(cali_reg[0]);

    //select the 5G,  Read RF calibration register values
    BB_WriteReg(PAGE0, 0x64, (data | 0x80));
    read_cali_register(cali_reg[1]);
}



void BB_RF_band_switch(ENUM_RF_BAND rf_band)
{
    uint8_t data, data1;

    char *regbuf = ((rf_band==RF_2G) ? cali_reg[0]:cali_reg[1]); 

    //write 0xd0[7] -> 0x67[7]
    data = (BB_ReadReg(PAGE0, 0x67) & 0x7f) | (regbuf[0] & 0x80);
    BB_WriteReg(PAGE0, 0x67, data);

    //write 0xd0[5:2] -> 0x68[7:4]
    data1 = ((regbuf[0] & 0x3c) << 2);
    data = (BB_ReadReg(PAGE0, 0x68) & 0x0f) | data1;
    BB_WriteReg(PAGE0, 0x68, data);

    //write 0xd1[7] -> 0x67[6]
    data = (BB_ReadReg(PAGE0, 0x67) & 0xbf) |  ((regbuf[1] & 0x80) >> 1);
    BB_WriteReg(PAGE0, 0x67, data);

    //write 0xd1[5:2] -> 0x68[3:0]
    data1 = ((regbuf[1] & 0x3c) >> 2);
    data = (BB_ReadReg(PAGE0, 0x68) & 0xf0) | data1;
    BB_WriteReg(PAGE0, 0x68, data);
    
    //write 0xd4[7] -> 0x67[5]
    data = (BB_ReadReg(PAGE0, 0x67) &  0xdf) | ((regbuf[4] & 0x80) >> 2);
    BB_WriteReg(PAGE0, 0x67, data);

    //write 0xd4[6:3] -> 0x6a[7:4]
    data1 = ((regbuf[4] & 0x78) << 1);
    data = (BB_ReadReg(PAGE0, 0x6A) & 0x0f) | data1;
    BB_WriteReg(PAGE0, 0x6A, data);
    
    //0xd3[3:0]->0x6a[3:0]
    data = (BB_ReadReg(PAGE0, 0x6A) & 0xf0) | (regbuf[3] & 0x0f);;
    BB_WriteReg(PAGE0, 0x6A, data);
    
    //0xd2[7:0] -> 0x6b[7:0]
    BB_WriteReg(PAGE0, 0x6b, regbuf[2]);
    
    uint16_t tmp = (((uint16_t)regbuf[3] & 0x0f) << 8 ) | regbuf[2]; //abs(tmp[11:0])
    if(tmp & 0x800) //tmp[11]
    {
        tmp = (~tmp) + 1;
        tmp &= 0x0fff;
    }

    typedef struct _STRU_thresh_regvalue
    {
        uint16_t thresh;
        uint8_t value;
    }STRU_thresh_regvalue;

    STRU_thresh_regvalue thresh_regvalue[] = 
    {
        {0x41,  0xFF}, {0x60,  0xFE}, {0x70,  0xFD}, {0x80,  0xFC}, 
        {0x8F,  0xFB}, {0x9F,  0xFA}, {0xAF,  0xF8}, {0xBE,  0xF7}, 
        {0xCE,  0xF6}, {0xDD,  0xF4}, {0xED,  0xF2}, {0xFD,  0xF0}, 
        {0x10C, 0xEE}, {0x11C, 0xEC}, {0x12B, 0xEA}, {0x13B, 0xE7}, 
        {0x14A, 0xE5}, {0x15A, 0xE2}, {0x169, 0xE0}, {0x179, 0xDD}, 
        {0x188, 0xDA}, {0x198, 0xD7}, {0x1A7, 0xD4}, {0x1B6, 0xD0},
        {0x1C6, 0xCD}, {0x1D5, 0xC9},
    };

    uint8_t regvalue = 0xc6;
    uint8_t idx = 0;
    for(idx = 0; idx < sizeof(thresh_regvalue)/sizeof(thresh_regvalue[0]); idx++)
    {
        if(tmp <= thresh_regvalue[idx].thresh)
        {
            regvalue = thresh_regvalue[idx].value;
            break;
        }
    }

    BB_WriteReg(PAGE0, 0x69, regvalue);

    //write 0xd5[7] -> 0x67[3]
    data1 = (regbuf[5] & 0x80) >> 4;
    data = BB_ReadReg(PAGE0, 0x67) & 0xf7 | data1;
    BB_WriteReg(PAGE0, 0x67, data);
    
    //0xd5[5:2]->0x67[7:4]
    data1 = (regbuf[5] & 0x3c) << 2;
    data = (BB_ReadReg(PAGE0, 0x6c) & 0x0f) | data1;
    BB_WriteReg(PAGE0, 0x6c, data);

    //write 0xd6[7] -> 0x67[2]
    data1 = (regbuf[6] & 0x80) >> 5;
    data = (BB_ReadReg(PAGE0, 0x67) & 0xfb) | data1;
    BB_WriteReg(PAGE0, 0x67, data);

    //0xd6[5:2]->0x6c[3:0]
    data1 = (regbuf[6] >> 2) & 0x0f ;
    data = (BB_ReadReg(PAGE0, 0x6c) & 0xf0) | data1;
    BB_WriteReg(PAGE0, 0x6c, data);

    //write 0xd9[7] -> 0x67[1]
    data1 = (regbuf[9] & 0x80) >> 6;
    data = (BB_ReadReg(PAGE0, 0x67) & 0xfd) | data1;
    BB_WriteReg(PAGE0, 0x67, data);
    
    //write 0xd9[6:3] -> 0x6E[7:4]
    data1 = (regbuf[9]<<1) & 0xf0;
    data = (BB_ReadReg(PAGE0, 0x6e) & 0x0f) | data1;
    BB_WriteReg(PAGE0, 0x6e, data); 

    //0xd8[3:0] -> 0x6E[3:0]
    data1 = regbuf[8] & 0x0f;
    data = (BB_ReadReg(PAGE0, 0x6e) & 0xf0) | data1;
    BB_WriteReg(PAGE0, 0x6e, data);

    //0xd7[7:0] -> 0x6F[7:0]
    BB_WriteReg(PAGE0, 0x6f, regbuf[7]);

    tmp = (((uint16_t)regbuf[8] & 0x0f)<<8) | regbuf[7];
    if(tmp & 0x800) //tmp[11]
    {
        tmp = (~tmp) + 1;
        tmp &= 0x0fff;
    }

    regvalue = 0xc6;
    for(idx = 0; idx < sizeof(thresh_regvalue)/sizeof(thresh_regvalue[0]); idx++)
    {
        if(tmp <= thresh_regvalue[idx].thresh)
        {
            regvalue = thresh_regvalue[idx].value;
            break;
        }
    }

    BB_WriteReg(PAGE0, 0x6d, regvalue);
    BB_WriteRegMask(PAGE0, 0x60, 0x02, 0x02);   //fix calibration result.

#if 0
    data = BB_ReadReg(PAGE0, 0x00);
    data |= 0x01;
    BB_WriteReg(PAGE0, 0x00, data);
    
    data &= 0xfe;
    BB_WriteReg(PAGE0, 0x00, data);  
#endif

    BB_WriteReg(PAGE2, RF_BAND_CHANGE_0, rf_band);
    BB_WriteReg(PAGE2, RF_BAND_CHANGE_1, rf_band);    
}


void RF_CaliProcess(ENUM_BB_MODE en_mode, STRU_BoardCfg *boardCfg)
{
    BB_before_RF_cali();

    BB_RF_start_cali();

    BB_WriteReg(PAGE0, TX_CALI_ENABLE, 0x00);   //disable calibration

    RF8003s_afterCali(en_mode, boardCfg);
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
    STRU_FRQ_CHANNEL *pstru_frq = ((band == RF_2G)?It_2G_frq:It_5G_frq);

    BB_WriteReg(PAGE2, IT_FRQ_0, pstru_frq[u8_ch].frq1);
    BB_WriteReg(PAGE2, IT_FRQ_1, pstru_frq[u8_ch].frq2);
    BB_WriteReg(PAGE2, IT_FRQ_2, pstru_frq[u8_ch].frq3);
    BB_WriteReg(PAGE2, IT_FRQ_3, pstru_frq[u8_ch].frq4);
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


uint8_t BB_set_ItFrqByCh(ENUM_RF_BAND band, uint8_t ch)
{
    STRU_FRQ_CHANNEL *it_ch_ptr = ((band == RF_2G)?It_2G_frq:It_5G_frq);

    context.stru_itRegs.frq1 = it_ch_ptr[ch].frq1;
    context.stru_itRegs.frq2 = it_ch_ptr[ch].frq2;
    context.stru_itRegs.frq3 = it_ch_ptr[ch].frq3;
    context.stru_itRegs.frq4 = it_ch_ptr[ch].frq4;

    BB_WriteReg(PAGE2, AGC3_0, it_ch_ptr[ch].frq1);
    BB_WriteReg(PAGE2, AGC3_1, it_ch_ptr[ch].frq2);
    BB_WriteReg(PAGE2, AGC3_2, it_ch_ptr[ch].frq3);
    BB_WriteReg(PAGE2, AGC3_3, it_ch_ptr[ch].frq4);
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

    STRU_FRQ_CHANNEL *pu8_rcRegs = ((band == RF_2G)?Rc_2G_frq:Rc_5G_frq);

    context.stru_rcRegs.frq1 = pu8_rcRegs[ch].frq1;
    context.stru_rcRegs.frq2 = pu8_rcRegs[ch].frq2;
    context.stru_rcRegs.frq3 = pu8_rcRegs[ch].frq3;
    context.stru_rcRegs.frq4 = pu8_rcRegs[ch].frq4;

    BB_WriteReg(PAGE2, AGC3_a, pu8_rcRegs[ch].frq1);
    BB_WriteReg(PAGE2, AGC3_b, pu8_rcRegs[ch].frq2);
    BB_WriteReg(PAGE2, AGC3_c, pu8_rcRegs[ch].frq3);
    BB_WriteReg(PAGE2, AGC3_d, pu8_rcRegs[ch].frq4); 

}



uint8_t BB_set_SweepFrq(ENUM_RF_BAND band, ENUM_CH_BW e_bw, uint8_t ch)
{
    STRU_FRQ_CHANNEL *ch_ptr;

    if (BW_10M == e_bw)
    {
        ch_ptr = ((band == RF_2G)?Sweep_2G_10m_frq:Sweep_5G_10m_frq);
    }
    else
    {
        ch_ptr = ((band == RF_2G)?Sweep_2G_20m_frq:Sweep_5G_20m_frq);
    }

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

