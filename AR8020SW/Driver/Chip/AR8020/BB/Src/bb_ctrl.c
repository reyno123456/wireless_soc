#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "debuglog.h"
#include "bb_spi.h"
#include "bb_ctrl_internal.h"
#include "bb_grd_sweep.h"
#include "bb_grd_ctrl.h"
#include "bb_sky_ctrl.h"
#include "bb_uart_com.h"
#include "reg_rw.h"
#include "systicks.h"
#include "bb_regs.h"
#include "sys_event.h"
#include "rf_if.h"
#include "memory_config.h"
#include "boardParameters.h"
#include "cfg_parser.h"

#define     BB_SPI_TEST         (0)
#define     RF_SPI_TEST         (0)

#define     VSOC_GLOBAL2_BASE   (0xA0030000)
#define     BB_SPI_UART_SEL     (0x9c)

volatile CONTEXT context;
static volatile ENUM_REG_PAGES en_curPage;

uint8_t *p_bbRegs = NULL;

STRU_BOARD_BB_PARA *pstru_bb_boardcfg= NULL;

STRU_BB_REG *p_bb_reg_beforeCali = NULL;
STRU_BB_REG *p_bb_reg_afterCali  = NULL;


static int BB_before_RF_cali(void);
static void BB_GetRcIdFromFlash(uint8_t *pu8_rcid);
static void BB_after_RF_cali(ENUM_BB_MODE en_mode);
static void BB_RF_start_cali( void );
static void BB_HandleEventsCallback(void *p);



static void BB_getCfgData(ENUM_BB_MODE en_mode, STRU_cfgBin *cfg)
{
    STRU_cfgNode  *p_bb_regNode;

    //BB init registers
    p_bb_regNode = CFGBIN_GetNode(cfg, (en_mode==BB_SKY_MODE) ? BB_SKY_REG_INIT_NODE_ID : BB_GRD_REG_INIT_NODE_ID);
    if (NULL != p_bb_regNode)
    {
        p_bbRegs  = (uint8_t *)(p_bb_regNode + 1);
    }

    //BB board param
    p_bb_regNode = CFGBIN_GetNode(cfg, BB_BOARDCFG_PARA_ID);
    if (NULL != p_bb_regNode)
    {
        pstru_bb_boardcfg  = (STRU_BOARD_BB_PARA *)(p_bb_regNode + 1);
    }

    STRU_cfgNode *p_bb_dataNode = CFGBIN_GetNode(cfg, BB_BOARDCFG_DATA_ID);
    if (NULL != p_bb_dataNode)
    {
        if (en_mode == BB_SKY_MODE)
        {
            p_bb_reg_beforeCali = (STRU_BB_REG *)(p_bb_dataNode + 1);
            p_bb_reg_afterCali  = p_bb_reg_beforeCali + pstru_bb_boardcfg->u8_bbSkyRegsCnt + pstru_bb_boardcfg->u8_bbGrdRegsCnt;
        }
        else
        {
            p_bb_reg_beforeCali = (STRU_BB_REG *)(p_bb_dataNode + 1) + pstru_bb_boardcfg->u8_bbSkyRegsCnt;
            p_bb_reg_afterCali  = p_bb_reg_beforeCali + pstru_bb_boardcfg->u8_bbGrdRegsCnt + pstru_bb_boardcfg->u8_bbSkyRegsCntAfterCali;
        }
    }

    //DLOG_Error("%x %x ", p_bbRegs[0], p_bbRegs[1]);
    //DLOG_Error("%x %x %x", p_bb_reg_beforeCali->page, p_bb_reg_beforeCali->addr, p_bb_reg_beforeCali->value);
    //DLOG_Error("%x %x %x", p_bb_reg_afterCali->page, p_bb_reg_afterCali->addr, p_bb_reg_afterCali->value);
}


static void BB_regs_init(ENUM_BB_MODE en_mode)
{
    uint32_t page_cnt = 0;
    uint8_t  regsize;

    if (NULL != pstru_bb_boardcfg)
    {
        regsize = (en_mode == BB_SKY_MODE) ? pstru_bb_boardcfg->u8_bbSkyRegsCnt : pstru_bb_boardcfg->u8_bbGrdRegsCnt;
    }

    //update the board registers
    {
        uint8_t num;
        for(num = 0; num < regsize; num++)
        {
            uint16_t addr  = ((uint16_t)p_bb_reg_beforeCali[num].page << 8) + p_bb_reg_beforeCali[num].addr;
            uint8_t  value = p_bb_reg_beforeCali[num].value;
            p_bbRegs[addr] = value;
        }
    }

    for(page_cnt = 0 ; page_cnt < 4; page_cnt ++)
    {
        uint32_t addr_cnt=0;
        ENUM_REG_PAGES page = (ENUM_REG_PAGES)(page_cnt << 6);
        /*
         * PAGE setting included in the regs array.
         */
        en_curPage = page;

        for(addr_cnt = 0; addr_cnt < 256; addr_cnt++)
        {
            //PAGE1 reg[0xa1] reg[0xa2] reg[0xa4] reg[0xa5] are PLL setting for cpu0, cpu1, cpu2, set in the sysctrl.c when system init
            if(page==PAGE1 && (addr_cnt==0xa1||addr_cnt==0xa2||addr_cnt==0xa4||addr_cnt==0xa5))
            {}
            else
            {
                BB_SPI_curPageWriteByte((uint8_t)addr_cnt, *p_bbRegs);
            }
            p_bbRegs++;
        }
    }
}


int BB_softReset(ENUM_BB_MODE en_mode)
{
    uint8_t reg_after_reset;
    if(en_mode == BB_GRD_MODE)
    {
        BB_SPI_curPageWriteByte(0x00,0xB1);
        BB_SPI_curPageWriteByte(0x00,0xB0);
        reg_after_reset = 0xB0;
    }
    else
    {        
        BB_SPI_curPageWriteByte(0x00, 0x81);
        BB_SPI_curPageWriteByte(0x00, 0x80);
        reg_after_reset = 0x80;
    }

    //bug fix: write reset register may fail. 
    int count = 0;
    while(count++ < 5)
    {
        uint8_t rst = BB_SPI_curPageReadByte(0x00);
        if(rst != reg_after_reset)
        {
            BB_SPI_curPageWriteByte(0x00, reg_after_reset);
            count ++;
        }
        else
        {
            break;
        }
    }

    if (count >= 5)
    {
        dlog_warning("Reset Error");
    }
    en_curPage = PAGE2;
    return 0;
}


STRU_CUSTOMER_CFG stru_defualtCfg = 
{
    .enum_chBandWidth = BW_10M,
    .flag_useCfgId    = 0,
    .itHopMode        = AUTO,
    .rcHopMode        = AUTO,
    .qam_skip_mode    = AUTO,
};

void BB_init(ENUM_BB_MODE en_mode, STRU_CUSTOMER_CFG *pstru_customerCfg)
{
    if (NULL==pstru_customerCfg)
    {
        pstru_customerCfg = &stru_defualtCfg;
    }

    BB_getCfgData(en_mode, (STRU_cfgBin *)SRAM_CONFIGURE_MEMORY_ST_ADDR);

    context.en_bbmode = en_mode;
    context.itHopMode = pstru_customerCfg->itHopMode;
    context.rcHopMode = pstru_customerCfg->rcHopMode;
    context.qam_skip_mode = pstru_customerCfg->qam_skip_mode;
    context.e_bandwidth   = pstru_customerCfg->enum_chBandWidth;

    context.e_bandsupport = pstru_bb_boardcfg->e_bandsupport;

    //get rc id from  1) user cfg
    //                2) saved rc id
    if( 1 == pstru_customerCfg->flag_useCfgId )
    {
        memcpy( (void *)context.rcid, (void *)(pstru_customerCfg->rcid), RC_ID_SIZE);
    }
    else
    {
        BB_GetRcIdFromFlash((uint8_t *)context.rcid);
    }

    BB_uart10_spi_sel(0x00000003);
    BB_SPI_init();

    context.u8_bbStartMcs = ((context.e_bandwidth == BW_20M) ? (pstru_bb_boardcfg->u8_bbStartMcs20M) : (pstru_bb_boardcfg->u8_bbStartMcs10M));

    BB_regs_init(context.en_bbmode);
    RF_init(en_mode);
    RF_CaliProcess(en_mode);

    BB_after_RF_cali(en_mode);

    BB_softReset(en_mode);
    //choose the start band
    if (context.e_bandsupport == RF_2G_5G || context.e_bandsupport == RF_2G)
    {
        context.e_curBand = RF_2G;
    }
    else if( context.e_bandsupport == RF_5G )
    {
        context.e_curBand = RF_5G;
    }
    else
    {
        dlog_info("RF band: 600MHz");
        context.e_curBand = RF_600M;
    }

    BB_set_RF_Band(en_mode, context.e_curBand);
    BB_set_RF_bandwitdh(en_mode, context.e_bandwidth);
    BB_softReset(en_mode);

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_USER_CFG_CHANGE, BB_HandleEventsCallback);
    dlog_warning("use board cfg:%d %d %d", context.e_bandwidth, context.e_curBand, context.e_bandsupport);
}


void BB_uart10_spi_sel(uint32_t sel_dat)
{
    write_reg32( (uint32_t *)(VSOC_GLOBAL2_BASE + BB_SPI_UART_SEL),	sel_dat);
}


uint8_t BB_WriteReg(ENUM_REG_PAGES page, uint8_t addr, uint8_t data)
{
    if(en_curPage != page)
    {
        BB_SPI_WriteByte(page, addr, data);
        en_curPage = page;
    }
    else
    {
        BB_SPI_curPageWriteByte(addr, data);
    }
}

uint8_t BB_ReadReg(ENUM_REG_PAGES page, uint8_t addr)
{
    uint8_t reg;
    if(en_curPage != page)
    {
        reg = BB_SPI_ReadByte(page, addr);
        en_curPage = page;
    }
    else
    {
        reg = BB_SPI_curPageReadByte(addr);
    }
    return reg;
}

int BB_WriteRegMask(ENUM_REG_PAGES page, uint8_t addr, uint8_t data, uint8_t mask)
{
    uint8_t ori = BB_ReadReg(page, addr);
    data = (ori & (~mask)) | data;
    return BB_WriteReg(page, addr, data);
}


int BB_ReadRegMask(ENUM_REG_PAGES page, uint8_t addr, uint8_t mask)
{
    return BB_ReadReg(page, addr) & mask;
}



const uint8_t mcs_idx_bitrate_map_10m[] = 
{
    1,      //0.6Mbps BPSK 1/2
    2,      //1.2     BPSK 1/2
    3,      //2.4     QPSK 1/2
    8,      //5.0     16QAM 1/2
    11,     //7.5     64QAM 1/2
    13,     //10      64QAM 2/3
};

const uint8_t mcs_idx_bitrate_map_20m[] = 
{
    2,      //1.2Mbps BPSK 1/2
    3,      //2.4     BPSK 1/2
    8,      //5.0     QPSK 1/2
    11,      //7.5     16QAM 1/2
    13,     //10     16QAM 2/3
};

uint8_t BB_get_bitrateByMcs(ENUM_CH_BW bw, uint8_t u8_mcs)
{
    if (BW_10M == bw)
    {
        return mcs_idx_bitrate_map_10m[u8_mcs];
    }
    else
    {
        return mcs_idx_bitrate_map_20m[u8_mcs];
    }
}



void BB_saveRcid(uint8_t *u8_idArray)
{
    STRU_SysEvent_NvMsg st_nvMsg;

    // src:cpu0 dst:cpu2
    st_nvMsg.u8_nvSrc = INTER_CORE_CPU2_ID;
    st_nvMsg.u8_nvDst = INTER_CORE_CPU0_ID;

    // parament number
    st_nvMsg.e_nvNum = NV_NUM_RCID;

    // parament set
    st_nvMsg.u8_nvPar[0] = 1;
    memcpy(&(st_nvMsg.u8_nvPar[1]), u8_idArray, 5);

    // send msg
    SYS_EVENT_Notify(SYS_EVENT_ID_NV_MSG, (void *)(&(st_nvMsg)));
}

void BB_set_QAM(ENUM_BB_QAM mod)
{
    uint8_t data = BB_ReadReg(PAGE2, TX_2);
    BB_WriteReg(PAGE2, TX_2, (data & 0x3f) | ((uint8_t)mod << 6));
}

void BB_set_LDPC(ENUM_BB_LDPC ldpc)
{
    uint8_t data = BB_ReadReg(PAGE2, TX_2);
    BB_WriteReg(PAGE2, TX_2, (data & 0xF8) | (uint8_t)ldpc);
}


/************************************************************
PAGE2	0x20[2]	rf_freq_sel_rx_sweep    RW    sweep frequency selection for the 2G frequency band o or 5G frequency band,
		0'b0: 2G frequency band
		1'b1: 5G frequency band

PAGE2	0x21[7]	rf_freq_sel_tx	          RW     The frequency band selection for the transmitter
		1'b0: 2G frequency band
		1'b1 for 5G frequency band

PAGE2	0x21[4]	rf_freq_sel_rx_work	   RW    The frequency band selection for the receiver
		1'b0: 2G frequency band
		1'b1 for 5G frequency band
*****************************************************************/
/*
 * set RF baseband to 2.4G or 5G
 */
void BB_set_RF_Band(ENUM_BB_MODE sky_ground, ENUM_RF_BAND rf_band)
{
    if(rf_band == RF_2G)
    {
        BB_WriteRegMask(PAGE0, 0x20, 0x08, 0x0c);
        BB_WriteRegMask(PAGE2, 0x21, 0x00, 0x90);
        BB_WriteRegMask(PAGE2, 0x20, 0x00, 0x04);
    }
    else if(rf_band == RF_5G)
    {
        // P0 0x20 [3]=0, [2]=1,2G PA off,5G PA on
        BB_WriteRegMask(PAGE0, 0x20, 0x04, 0x0C); 
        // P2 0x21 [7]=1, [4]=1,rf_freq_sel_tx,rf_freq_sel_rx,5G
        BB_WriteRegMask(PAGE2, 0x21, 0x90, 0x90); 
        // P2 0x20 [2]=1,sweep frequency,5G
        BB_WriteRegMask(PAGE2, 0x20, 0x04, 0x04);

        //softreset
        //BB_softReset(sky_ground);

    }

    //calibration and reset
    BB_RF_band_switch(rf_band);
}


/*
 * set RF bandwidth = 10M or 20M
*/
void BB_set_RF_bandwitdh(ENUM_BB_MODE sky_ground, ENUM_CH_BW rf_bw)
{
    if (sky_ground == BB_SKY_MODE)
    {
        BB_WriteRegMask(PAGE2, TX_2, (rf_bw << 3), 0x38); /*bit[5:3]*/
        if (BW_20M == (context.e_bandwidth))
        {
            BB_WriteRegMask(PAGE2, 0x05, 0x80, 0xC0);
        }
    }
    else
    {
        BB_WriteRegMask(PAGE2, RX_MODULATION, (rf_bw << 0), 0x07); /*bit[2:0]*/
    }
   
    //softreset
    BB_softReset(sky_ground);
}



static void BB_after_RF_cali(ENUM_BB_MODE en_mode)
{
    //BB_WriteRegMask(PAGE0, 0x20, 0x80, 0x80);
    // enalbe RXTX
    //BB_WriteRegMask(PAGE1, 0x94, 0x10, 0xFF);    //remove to fix usb problem

    uint8_t bb_regcnt = (en_mode == BB_SKY_MODE) ? pstru_bb_boardcfg->u8_bbSkyRegsCntAfterCali : pstru_bb_boardcfg->u8_bbGrdRegsCntAfterCali;

    if ( bb_regcnt > 0)
    {
        uint8_t cnt;
        for(cnt = 0; cnt < bb_regcnt; cnt ++)
        {
            ENUM_REG_PAGES page = (ENUM_REG_PAGES )(p_bb_reg_afterCali[cnt].page << 6);
            BB_WriteReg(page, p_bb_reg_afterCali[cnt].addr, p_bb_reg_afterCali[cnt].value);
        }
    }
}


typedef struct _STRU_grd_cmds
{
    uint8_t avail;                      /*command is using*/
    STRU_WIRELESS_CONFIG_CHANGE config;
}STRU_grd_cmds;


static STRU_grd_cmds grd_cmds_poll[16];

/*
 * BB_Getcmd: get command from command buffer pool and free the buffer
*/
int BB_GetCmd(STRU_WIRELESS_CONFIG_CHANGE *pconfig)
{
    uint8_t found = 0;
    uint8_t i;
    for(i = 0; i < sizeof(grd_cmds_poll)/sizeof(grd_cmds_poll[0]); i++)
    {
        if(grd_cmds_poll[i].avail == 1)
        {
            memcpy(pconfig, &(grd_cmds_poll[i].config), sizeof(*pconfig));
            grd_cmds_poll[i].avail = 0;
            found = 1;
            break;
        }
    }

    return (found) ? TRUE:FALSE;
}


int BB_InsertCmd(STRU_WIRELESS_CONFIG_CHANGE *p)
{
    uint8_t i;
    uint8_t found = 0;
    STRU_WIRELESS_CONFIG_CHANGE *pcmd = (STRU_WIRELESS_CONFIG_CHANGE *)p;

    //dlog_info("Insert Message: %d %d %d\r\n", pcmd->u8_configClass, pcmd->u8_configItem, pcmd->u32_configValue);
    for(i = 0; i < sizeof(grd_cmds_poll)/sizeof(grd_cmds_poll[0]); i++)
    {
        if(grd_cmds_poll[i].avail == 0)
        {
            memcpy((void *)(&grd_cmds_poll[i].config), p, sizeof(grd_cmds_poll[0]));
            grd_cmds_poll[i].avail = 1;
            found = 1;
            break;
        }
    }

    if(!found)
    {
        dlog_warning("ERROR:Insert Event");
    }

    return (found? TRUE:FALSE);
}


int BB_add_cmds(uint8_t type, uint32_t param0, uint32_t param1, uint32_t param2, uint32_t param3)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    int ret = 1;
    switch(type)
    {
        case 0:
        {        
            cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
            cmd.u8_configItem   = FREQ_BAND_WIDTH_SELECT;
            cmd.u32_configValue  = param0;
            break;
        }

        case 1:
        {
            cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
            cmd.u8_configItem   = FREQ_BAND_MODE;
            cmd.u32_configValue  = param0;
            break;            
        }
        
        case 2:
        {
            cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
            cmd.u8_configItem   = FREQ_BAND_SELECT;
            cmd.u32_configValue  = param0;
            break;
        }

        case 3:
        {
            cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
            cmd.u8_configItem   = FREQ_CHANNEL_MODE;
            cmd.u32_configValue  = param0;
            break;
        }
    
        case 4:
        {
            cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
            cmd.u8_configItem   = FREQ_CHANNEL_SELECT;
            cmd.u32_configValue  = param0;
            break;
        }

        case 5:        
        {
            cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
            cmd.u8_configItem   = MCS_MODE_SELECT;
            cmd.u32_configValue  = param0;
            break;
        }

        case 6:
        {
            cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
            cmd.u8_configItem   = MCS_MODULATION_SELECT;
            cmd.u32_configValue  = param0;
            break;            
        }

        case 7:
        {
            cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
            cmd.u8_configItem   = MCS_CODE_RATE_SELECT;
            cmd.u32_configValue  = param0;
            break;
        }

        case 8:
        {
            cmd.u8_configClass  = WIRELESS_ENCODER_CHANGE;
            cmd.u8_configItem   = ENCODER_DYNAMIC_BIT_RATE_MODE;
            cmd.u32_configValue  = param0;
            break;
        }

        case 9:
        {
            cmd.u8_configClass  = WIRELESS_ENCODER_CHANGE;
            cmd.u8_configItem   = ENCODER_DYNAMIC_BIT_RATE_SELECT_CH1;
            cmd.u32_configValue  = param0;
            break;
        }

        case 10:
        {
            cmd.u8_configClass  = WIRELESS_ENCODER_CHANGE;
            cmd.u8_configItem   = ENCODER_DYNAMIC_BIT_RATE_SELECT_CH2;
            cmd.u32_configValue = param0;
            break;
        }

        case 11:
        {
            cmd.u8_configClass  = WIRELESS_MISC;
            cmd.u8_configItem   = MISC_READ_RF_REG;
            cmd.u32_configValue = (param0) | (param1 << 8) | (param2 << 16);

            dlog_info("1:%d 2:%d 3:%d 4:%d", type, param0, param1, param2);
            break;
        }

        case 12:
        {
            cmd.u8_configClass  = WIRELESS_MISC;
            cmd.u8_configItem   = MISC_WRITE_RF_REG;
                               //8003s num: addr: value
            cmd.u32_configValue  = (param0) | (param1 << 8) | (param2 << 16) | (param3 << 24);
            break;
        }

        case 13:
        {
            cmd.u8_configClass  = WIRELESS_MISC;
            cmd.u8_configItem   = MISC_READ_BB_REG;
                               //page, addr
            cmd.u32_configValue  = param0 | (param1 << 8);
            break;
        }
        
        case 14:
        {
            cmd.u8_configClass  = WIRELESS_MISC;
            cmd.u8_configItem   = MISC_WRITE_BB_REG;
                               //page, addr, value
            cmd.u32_configValue  = (param0) | (param1<<8) | (param2<<16);
            break;
        }

        case 15:
        {
            cmd.u8_configClass  = WIRELESS_DEBUG_CHANGE;
            cmd.u8_configItem   = 1;
            break;
        }

        case 16:
        {
            cmd.u8_configClass  = WIRELESS_AUTO_SEARCH_ID;
            cmd.u8_configItem   = 0;
            break;
        }
        
        case 17:
        {
            cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
            cmd.u8_configItem   = MCS_IT_QAM_SELECT;
            cmd.u32_configValue  = (param0);
            break;
        }
        
        case 18:
        {
            cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
            cmd.u8_configItem   = MCS_IT_CODE_RATE_SELECT;
            cmd.u32_configValue  = (param0);
            break;
        }

        case 19:
        {
            cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
            cmd.u8_configItem   = MCS_RC_QAM_SELECT;
            cmd.u32_configValue  = (param0);
            break;
        }
        
        case 20:
        {
            cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
            cmd.u8_configItem   = MCS_RC_CODE_RATE_SELECT;
            cmd.u32_configValue  = (param0);
            break;
        }
        
        case 21:
        {
            cmd.u8_configClass  = WIRELESS_OTHER;
            cmd.u8_configItem   = CALC_DIST_ZERO_CALI;
            cmd.u32_configValue  = (param0);
            break;
        }

        case 22:
        {
            cmd.u8_configClass  = WIRELESS_OTHER;
            cmd.u8_configItem   = SET_CALC_DIST_ZERO_POINT;
            cmd.u32_configValue  = (param0);
            break;
        }

        case 23:
        {
            cmd.u8_configClass  = WIRELESS_OTHER;
            cmd.u8_configItem   = SET_RC_FRQ_MASK;
            cmd.u32_configValue  = (param0);
            break;
        }
        
        case 24:
        {
            cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
            cmd.u8_configItem   = IT_CHANNEL_FREQ;
            cmd.u32_configValue  = (param0);
            break;
        }

        case 25:
        {
            cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
            cmd.u8_configItem   = MCS_CHG_RC_RATE;
            cmd.u32_configValue  = (param0);
            break;
        }
        
        case 26:
        {
            dlog_info("rc rate:%d",BB_GetRcRate(param0));
            ret = 0;
            break;
        }
        
        default:
        {
            ret = 0;
            break;
        }
    }

    if(ret)
    {
       ret = BB_InsertCmd( &cmd);
    }

    return ret;
}


static void BB_HandleEventsCallback(void *p)
{
    STRU_WIRELESS_CONFIG_CHANGE* pcmd = (STRU_WIRELESS_CONFIG_CHANGE* )p;
    uint8_t  class  = pcmd->u8_configClass;
    uint8_t  item   = pcmd->u8_configItem;
    uint32_t value  = pcmd->u32_configValue;

    if( class == WIRELESS_DEBUG_CHANGE && item == 0 && (value == 0 || value == 1))
    {    
        uint8_t u8_debugMode = ((value == 0) ? TRUE:FALSE);

        if( context.u8_debugMode != u8_debugMode )
        {
            context.u8_flagdebugRequest = u8_debugMode | 0x80;
            BB_SPI_curPageWriteByte(0x01, 0x02);
            en_curPage = (BB_SPI_curPageReadByte(0x0) & 0xc0);
        }
        dlog_info("Event Debug: %d \n", u8_debugMode);         
    }
    else
    {
        int ret = BB_InsertCmd( (STRU_WIRELESS_CONFIG_CHANGE * )p);
    }
}


void BB_handle_misc_cmds(STRU_WIRELESS_CONFIG_CHANGE* pcmd)
{
    uint8_t class = pcmd->u8_configClass;
    uint8_t item  = pcmd->u8_configItem;

    uint8_t value  = (uint8_t)(pcmd->u32_configValue);
    uint8_t value1 = (uint8_t)(pcmd->u32_configValue >> 8);
    uint8_t value2 = (uint8_t)(pcmd->u32_configValue >> 16);
    uint8_t value3 = (uint8_t)(pcmd->u32_configValue >> 24);

    if(class == WIRELESS_MISC)
    {
        switch(item)
        {
            case MISC_READ_RF_REG:
            {
                uint8_t v;
                BB_SPI_curPageWriteByte(0x01, (value == 0)? 0x01 : 0x03);               //value2==0: write RF8003-0
                                                                                        //value2==1: write RF8003-1
                RF_SPI_ReadReg( (value1<<8 )|value2, &v);
                BB_SPI_curPageWriteByte(0x01,0x02);
                dlog_info("RF read %d addrH=0x%0.2x addrL:%0.2x out:0x%0.2x", value, value1, value2, v);
                break;
            }

            case MISC_WRITE_RF_REG:
            {
                BB_SPI_curPageWriteByte(0x01, (value == 0)? 0x01 : 0x03);              //value2==0: write RF8003-0
                                                                                       //value2==1: write RF8003-1
                RF_SPI_WriteReg( (value1 << 8) | value2, value3);
                BB_SPI_curPageWriteByte(0x01,0x02);

                dlog_info("RF write %d addr=0x%0.2x value=0x%0.2x", value, (value1 << 8) | value2, value3);
                break;
            }

            case MISC_READ_BB_REG:
            {
                uint8_t v = BB_ReadReg( (ENUM_REG_PAGES)value, (uint8_t)value1);
                dlog_info("BB read PAGE=0x%0.2x addr=0x%0.2x value=0x%0.2x", value, value1, v);
                break;
            }

            case MISC_WRITE_BB_REG:
            {
                BB_WriteReg((ENUM_REG_PAGES)value, (uint8_t)value1, (uint8_t)value2);
                dlog_info("BB write PAGE=0x%0.2x addr=0x%0.2x value=0x%0.2x", value, value1, value2);
                break;
            }

            case MICS_IT_ONLY_MODE:
            {
                BB_WriteReg(PAGE2, 0x02, 0x06);
                break;
            }
        }
    }
}

////////////////// handlers for WIRELESS_FREQ_CHANGE //////////////////

/** 
 * @brief       API for set channel Bandwidth 10M/20M, the function can only be called by cpu2
 * @param[in]   en_bw: channel bandwidth setting 10M/20M
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetFreqBandwidthSelection(ENUM_CH_BW en_bw)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
    cmd.u8_configItem   = FREQ_BAND_WIDTH_SELECT;
    cmd.u32_configValue  = (uint32_t)en_bw;

    return BB_InsertCmd(&cmd);
}



/** 
 * @brief       API for set frequency band (2G/5G) selection mode (ATUO / Manual), the function can only be called by cpu2
 * @param[in]   mode: selection mode (ATUO / Manual)
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetFreqBandSelectionMode(ENUM_RUN_MODE en_mode)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
    cmd.u8_configItem   = FREQ_BAND_MODE;
    cmd.u32_configValue  = (uint32_t)en_mode;

    return BB_InsertCmd(&cmd);
}



/** 
 * @brief       API for set frequency band (2G/5G), the function can only be called by cpu2
 * @param[in]   mode: selection mode (ATUO / Manual)
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetFreqBand(ENUM_RF_BAND band)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
    cmd.u8_configItem   = FREQ_BAND_SELECT;
    cmd.u32_configValue  = (uint32_t)band;

    return BB_InsertCmd(&cmd);
}



/** 
 * @brief       API for set IT(image transmit) channel selection RUN mode(AUTO/Manual). the function can only be called by cpu2
 * @param[in]   qam: the modulation QAM mode for image transmit.
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetITChannelSelectionMode(ENUM_RUN_MODE en_mode)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
    cmd.u8_configItem   = FREQ_CHANNEL_MODE;
    cmd.u32_configValue  = (uint32_t)en_mode;

    return BB_InsertCmd(&cmd);
}


/** 
 * @brief       API for set IT(image transmit) channel Number. the function can only be called by cpu2
 * @param[in]   channelNum: the current channel number int current Frequency band(2G/5G)
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetITChannel(uint8_t channelNum)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
    cmd.u8_configItem   = FREQ_CHANNEL_SELECT;
    cmd.u32_configValue  = channelNum;

    return BB_InsertCmd(&cmd);
}



////////////////// handlers for WIRELESS_MCS_CHANGE //////////////////

/** 
 * @brief       API for set MCS(modulation, coderate scheme) mode, the function can only be called by cpu2
 * @param[in]   mode: auto or manual selection.
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetMCSmode(ENUM_RUN_MODE en_mode)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
    cmd.u8_configItem   = MCS_MODE_SELECT;
    cmd.u32_configValue  = (uint32_t)en_mode;

    return BB_InsertCmd(&cmd);
}


/** 
 * @brief       API for set the image transmit QAM mode, the function can only be called by cpu2
 * @param[in]   qam: modulation qam mode
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetITQAM(ENUM_BB_QAM qam)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
    cmd.u8_configItem   = MCS_MODULATION_SELECT;
    cmd.u32_configValue  = (uint32_t)qam;

    return BB_InsertCmd(&cmd);
}


/** 
 * @brief       API for set the image transmit LDPC coderate, the function can only be called by cpu2
 * @param[in]   ldpc:  ldpc coderate 
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetITLDPC(ENUM_BB_LDPC ldpc)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;

    cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
    cmd.u8_configItem   = MCS_CODE_RATE_SELECT;
    cmd.u32_configValue  = (uint32_t)ldpc;

    return BB_InsertCmd(&cmd);
}


////////////////// handlers for WIRELESS_ENCODER_CHANGE //////////////////

/** 
 * @brief       API for set the encoder bitrate control mode, the function can only be called by cpu2
 * @param[in]   mode: auto or manual selection.
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetEncoderBrcMode(ENUM_RUN_MODE en_mode)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;

    cmd.u8_configClass  = WIRELESS_ENCODER_CHANGE;
    cmd.u8_configItem   = ENCODER_DYNAMIC_BIT_RATE_MODE;
    cmd.u32_configValue  = (uint32_t)en_mode;

    return BB_InsertCmd(&cmd);
}


/** 
 * @brief       API for set the encoder bitrate Unit:Mbps, the function can only be called by cpu2
 * @param[in]   bitrate_Mbps: select the bitrate unit: Mbps
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetEncoderBitrateCh1(uint8_t bitrate_Mbps)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;

    cmd.u8_configClass  = WIRELESS_ENCODER_CHANGE;
    cmd.u8_configItem   = ENCODER_DYNAMIC_BIT_RATE_SELECT_CH1;
    cmd.u32_configValue  = (uint32_t)bitrate_Mbps;

    return BB_InsertCmd(&cmd);
}

int BB_SetEncoderBitrateCh2(uint8_t bitrate_Mbps)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;

    cmd.u8_configClass  = WIRELESS_ENCODER_CHANGE;
    cmd.u8_configItem   = ENCODER_DYNAMIC_BIT_RATE_SELECT_CH2;
    cmd.u32_configValue  = (uint32_t)bitrate_Mbps;

    return BB_InsertCmd(&cmd);
}

static void BB_GetRcIdFromFlash(uint8_t *pu8_rcid)
{
    uint32_t loop = 0;
    uint8_t flag_found = 0;
    STRU_NV *pst_nv = (STRU_NV *)SRAM_NV_MEMORY_ST_ADDR;

    while( loop++ < 500 && 0 == flag_found )
    {
        if ( 0x23178546 != pst_nv->st_nvMng.u32_nvInitFlag)
        {
            SysTicks_DelayMS(20);
        }
        else
        {
            flag_found = 1;
        }
    }

    if (flag_found)
    {
        uint8_t sum = 0;
        if (pst_nv->st_nvMng.u8_nvVld == TRUE)
        {
            memcpy( (void *)pu8_rcid, (void *)(pst_nv->st_nvDataUpd.u8_nvBbRcId), RC_ID_SIZE);
        }
        else
        {
            dlog_warning("ERROR: Not Find rcid");
        }
    }
}

/** 
 * @brief       
 * @param   
 * @retval      
 * @note      
 */
int BB_GetDevInfo(void)
{ 
    uint8_t u8_data;
    STRU_DEVICE_INFO *pst_devInfo = (STRU_DEVICE_INFO *)(DEVICE_INFO_SHM_ADDR);

    pst_devInfo->messageId = 0x19;
    pst_devInfo->paramLen = sizeof(STRU_DEVICE_INFO);

    pst_devInfo->skyGround = context.en_bbmode;
    pst_devInfo->band = context.e_curBand;
    
    //pst_devInfo->bandWidth = context.CH_bandwidth;
    pst_devInfo->itHopMode = context.itHopMode;
    pst_devInfo->rcHopping = context.rcHopMode;
    pst_devInfo->adapterBitrate = context.qam_skip_mode;
    u8_data = BB_ReadReg(PAGE1, 0x8D);
    pst_devInfo->channel1_on = (u8_data >> 6) & 0x01;
    pst_devInfo->channel2_on = (u8_data >> 7) & 0x01;
    pst_devInfo->isDebug = context.u8_debugMode;
    if (BB_GRD_MODE == context.en_bbmode )
    {
        u8_data = BB_ReadReg(PAGE2, GRD_FEC_QAM_CR_TLV);
        pst_devInfo->itQAM = u8_data & 0x03;
        pst_devInfo->itCodeRate  = ((u8_data >>2) & 0x07);
        u8_data = BB_ReadReg(PAGE2, RX_MODULATION);
        pst_devInfo->bandWidth = (u8_data >> 0) & 0x07;
       
        u8_data = BB_ReadReg(PAGE2, TX_2);
        pst_devInfo->rcQAM = (u8_data >> 6) & 0x01;
        pst_devInfo->rcCodeRate = (u8_data >> 0) & 0x01;
    }
    else
    {
        u8_data = BB_ReadReg(PAGE2, TX_2);
        pst_devInfo->itQAM = (u8_data >> 6) & 0x03;
        pst_devInfo->bandWidth = (u8_data >> 3) & 0x07;
        pst_devInfo->itCodeRate  = ((u8_data >> 0) & 0x07);
        
        u8_data = BB_ReadReg(PAGE2, 0x09);
        pst_devInfo->rcQAM = (u8_data >> 0) & 0x01;
        pst_devInfo->rcCodeRate = (u8_data >> 2) & 0x01;
    }

    if(context.brc_mode == AUTO)
    {
        pst_devInfo->ch1Bitrates = context.qam_ldpc;
        pst_devInfo->ch2Bitrates = context.qam_ldpc;
    }
    else
    {
        pst_devInfo->ch1Bitrates = context.brc_bps[0];
        pst_devInfo->ch2Bitrates = context.brc_bps[1];
    }

    pst_devInfo->u8_itRegs[0] = context.stru_itRegs.frq1;
    pst_devInfo->u8_itRegs[1] = context.stru_itRegs.frq2;
    pst_devInfo->u8_itRegs[2] = context.stru_itRegs.frq3;
    pst_devInfo->u8_itRegs[3] = context.stru_itRegs.frq4;

    pst_devInfo->u8_rcRegs[0] = context.stru_rcRegs.frq1;
    pst_devInfo->u8_rcRegs[1] = context.stru_rcRegs.frq2;
    pst_devInfo->u8_rcRegs[2] = context.stru_rcRegs.frq3;
    pst_devInfo->u8_rcRegs[3] = context.stru_rcRegs.frq4;   

    //pst_devInfo->u8_startWrite = 0;
    //pst_devInfo->u8_endWrite = 1;    
}

/** 
 * @brief       
 * @param   
 * @retval      
 * @note      
 */
int BB_SwtichOnOffCh(uint8_t u8_ch, uint8_t u8_data)
{
    uint8_t u8_regVal;

    u8_regVal = BB_ReadReg(PAGE1, 0x8D);
    if ((0 == u8_ch) && (0 == u8_data))
    {
        u8_regVal &= ~(0x40); // channel1 
        BB_WriteReg(PAGE1, 0x8D, u8_regVal); 
    }
    else if ((0 == u8_ch) && (1 == u8_data))
    {
        u8_regVal |= (0x40); // channel1 
        BB_WriteReg(PAGE1, 0x8D, u8_regVal); 
    }
    else if ((1 == u8_ch) && (0 == u8_data))
    {
        u8_regVal &= ~(0x80); // channel2 
        BB_WriteReg(PAGE1, 0x8D, u8_regVal); 
    }
    else if ((1 == u8_ch) && (1 == u8_data))
    {
        u8_regVal |= (0x80); // channel2 
        BB_WriteReg(PAGE1, 0x8D, u8_regVal); 
    }
    else
    {
    }
}

/** 
 * @brief       
 * @param   
 * @retval      
 * @note      
 */
int BB_WrSpiChkFlag(void)
{
    BB_WriteReg(PAGE2, SPI_CHK1, 0x55);
    BB_WriteReg(PAGE2, SPI_CHK2, 0xAA);

    return 0;
}

/** 
 * @brief       
 * @param   
 * @retval      
 * @note      
 */
int BB_ChkSpiFlag(void)
{
    if ((0x55 == BB_ReadReg(PAGE2, SPI_CHK1)) &&
        (0xAA == BB_ReadReg(PAGE2, SPI_CHK2)))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int BB_GetRcId(uint8_t *pu8_rcId, uint8_t bufsize)
{
    if ( bufsize < RC_ID_SIZE)
    {
        return 1;
    }
    else
    {
        memcpy((void *)pu8_rcId, (void *)context.rcid, RC_ID_SIZE);
    }

    return 0;

}

/** 
 * @brief       get rc rate
 * @param       none
 * @retval      1: BPSK 1/2, uart max 32bytes
 *              2: QPSK 2/3, uart max 208bytes
 *              0: unknow qam/code_rate
 * @note        None
 */
uint32_t BB_GetRcRate(ENUM_BB_MODE en_mode)
{
    uint32_t ret = 0;
    uint8_t rate = 0;
    
    if (BB_SKY_MODE == en_mode)
    {
        rate = BB_ReadReg(PAGE2, 0x09) & 0x05;

        if (0 == rate)
        {
            ret = 1; // BPSK 1/2
        }
        else if(0x05 == rate)
        {
            ret = 2; // QPSK 2/3
        }
    }
    else if (BB_GRD_MODE == en_mode)
    {
        rate = BB_ReadReg(PAGE2, 0x04) & 0x41;

        if (0 == rate)
        {
            ret = 1; // BPSK 1/2
        }
        else if(0x41 == rate)
        {
            ret = 2; // QPSK 2/3
        }
    }

    return ret;
}
