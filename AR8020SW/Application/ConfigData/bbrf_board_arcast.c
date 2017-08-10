#include <string.h>
#include "boardParameters.h"

#include "cfg_parser.h"
#include "bb_types.h"


/////////////////////////////////////////////

extern STRU_BOARD_BB_PARA stru_arcast_bb_boardCfg;

STRU_cfgNode AR8020ARCAST_nodeInfo= 
{
    .nodeId       = BB_BOARDCFG_PARA_ID,
    .nodeElemCnt  = 1,
    .nodeDataSize = sizeof(stru_arcast_bb_boardCfg)
};


STRU_BOARD_BB_PARA stru_arcast_bb_boardCfg __attribute__ ((aligned (4)))= 
{
    //before calibration
    .u8_bbSkyRegsCnt    = 2,
    
    .u8_bbGrdRegsCnt    = 2,

    .e_bandsupport      = RF_2G,

    //after calibration
    .u8_bbSkyRegsCntAfterCali    = 5,
    
    .u8_bbGrdRegsCntAfterCali    = 5,

    .u8_bbStartMcs10M            = 3,
    .u8_bbStartMcs20M            = 2,
};


extern STRU_BB_REG stru_arcast_2t2r_bb_reg[14];

STRU_cfgNode EK_2t2r_bbdata_nodeInfo =
{
    .nodeId       = BB_BOARDCFG_DATA_ID,
    .nodeElemCnt  = 14,
    .nodeDataSize = sizeof(stru_arcast_2t2r_bb_reg)
};


STRU_BB_REG stru_arcast_2t2r_bb_reg[14] = 
{
    //sky arcast_bb_regs_beforeCali
    {0, 0xBE, 0x05},  //RF power for PA
    {0, 0xCE, 0x05},  //RF power for PA

    //ground arcast_bb_regs_beforeCali
    {0, 0xBE, 0x05},  //RF power for PA
    {0, 0xCE, 0x05},  //RF power for PA

    //pstru_bbSkyRegsAfterCali
    {1, 0x90, 0xF5},  //to power down SAR10 (from 0xF7) to save about 2mA current 
    {1, 0x91, 0x68},  //to power down SAR8_2 (from 0x78) to save about 2mA current   
    {1, 0x92, 0x03},  //to half ADC op current (from 0x00)
    {1, 0x96, 0x40},  //to half DAC op current (from 0x00)    
    {2, 0x03, 0x00},  //1T2R

    //arcast_bb_grdregs_afterCali
    {1, 0x90, 0xF5},  //to power down SAR10 (from 0xF7) to save about 2mA current 
    {1, 0x91, 0x68},  //to power down SAR8_2 (from 0x78) to save about 2mA current   
    {1, 0x92, 0x03},  //to half ADC op current (from 0x00)
    {1, 0x96, 0x40},  //to half DAC op current (from 0x00)      
    {2, 0x08, 0x52},   //1T2R
};



////////////////////////////////////////////////

extern STRU_BOARD_RF_PARA stru_arcast_rf_boardCfg;

STRU_cfgNode stru_arcast_rfcfg_nodeInfo =
{
    .nodeId       = RF_BOARDCFG_PARA_ID,
    .nodeElemCnt  = 1,
    .nodeDataSize = sizeof(stru_arcast_rf_boardCfg)
};


STRU_BOARD_RF_PARA stru_arcast_rf_boardCfg __attribute__ ((aligned (4)))= 
{
    //before calibration

    .u8_rfCnt           = 1,

    .u8_rf0SkyRegsCnt   = 5,
    
    .u8_rf0GrdRegsCnt   = 5,

    .u8_rf1GrdRegsCnt   = 0,

    //after calibration

    .u8_rf0SkyRegsCntAfterCali   = 7,
    
    .u8_rf0GrdRegsCntAfterCali   = 7,
    
    .u8_rf0GrdRegsCntAfterCali   = 0,

    .boardCfgName                = "ARCast"
};


extern STRU_RF_REG bbrf_ARCAST_regCfg[24];

STRU_cfgNode AR8020Test_nodeInfo =
{
    .nodeId       = RF_BOARDCFG_DATA_ID,    
    .nodeElemCnt  = 24,
    .nodeDataSize = sizeof(bbrf_ARCAST_regCfg)
};


STRU_RF_REG bbrf_ARCAST_regCfg[24] __attribute__ ((aligned (4)))= 
{
    //sky arcast_rf1_regs_beforeCali
    //improve RX NF
    {0, 0x01, 0x04},
    {0, 0x04, 0xc0},
    {0, 0x05, 0x0c},
    {0, 0x06, 0x80},

    {0, 0x33, 0x4c},       //internal PA: PA mode

    //ground arcast_rf1_regs_beforeCali
    //improve RX NF
    {0, 0x01, 0x04},
    {0, 0x04, 0xc0},
    {0, 0x05, 0x0c},
    {0, 0x06, 0x80},

    {0, 0x33, 0x4c},       //internal PA: PA mode

    //arcast_rf1_skyregs_afterCali
    {0, 0x25, 0x55},//to reduce current (about 4mA) of LO MXR (from 0x54)
    {0, 0x3B, 0x40},//to reduce TX LO current (about 3mA) by reducing LDO voltage (from 0x00)
    {0, 0x35, 0x76},//to reduce 2G PA Bias current (about 35mA), may degrade Pout 1~2db (from 0x70)
    {0, 0x45, 0x87},
    {0, 0x00, 0x74}, //1TX only
    {0, 0x2D, 0xF6},
    {0, 0x37, 0xE0},

    //arcast_rf1_grdregs_afterCali
    {0, 0x25, 0x55},//to reduce current (about 4mA) of LO MXR (from 0x54)
    {0, 0x3B, 0x40},//to reduce TX LO current (about 3mA) by reducing LDO voltage (from 0x00)
    {0, 0x35, 0x76},//to reduce 2G PA Bias current (about 35mA), may degrade Pout 1~2db (from 0x70)
    {0, 0x45, 0x87},
    {0, 0x00, 0x74}, //1TX only
    {0, 0x2D, 0xF6},
    {0, 0x37, 0xE0},
};
