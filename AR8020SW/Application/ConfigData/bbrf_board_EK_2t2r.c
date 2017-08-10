#include <string.h>
#include "boardParameters.h"
#include "cfg_parser.h"
#include "bb_types.h"


////////////////////////////////////////////////
extern STRU_BOARD_BB_PARA stru_EK_2t2r_bb_boardCfg;

STRU_cfgNode EK_2t2r_bbcfg_nodeInfo= 
{
    .nodeId       = BB_BOARDCFG_PARA_ID,
    .nodeElemCnt  = 1,    
    .nodeDataSize = sizeof(stru_EK_2t2r_bb_boardCfg)
};


STRU_BOARD_BB_PARA stru_EK_2t2r_bb_boardCfg __attribute__ ((aligned (4)))= 
{
    .u8_bbSkyRegsCnt             = 0,
    
    .u8_bbGrdRegsCnt             = 0,

    .e_bandsupport               = RF_2G_5G,

     //after calibration
    .u8_bbSkyRegsCntAfterCali    = 2,

    .u8_bbGrdRegsCntAfterCali    = 2,

    .u8_bbStartMcs10M            = 0,
    .u8_bbStartMcs20M            = 0,
};


extern STRU_BB_REG stru_EK_2t2r_bb_reg[4];

STRU_cfgNode EK_2t2r_bbdata_nodeInfo =
{
    .nodeId       = BB_BOARDCFG_DATA_ID,
    .nodeElemCnt  = 4,
    .nodeDataSize = sizeof(stru_EK_2t2r_bb_reg)
};


STRU_BB_REG stru_EK_2t2r_bb_reg[4] __attribute__ ((aligned (4)))= 
{
    //ek2T2R_bb_skyregsAfterCali
    {1, 0x90, 0xFF}, //for 2TX
    {1, 0x91, 0x78}, //2RX
    
    //ek2T2R_bb_grdregsAfterCali
    {1, 0x90, 0xF7}, //turn off DAC_B for 1TX
    {1, 0x91, 0x78}, //2RX
};



////////////////////////////////////////////////

extern STRU_BOARD_RF_PARA stru_EK_2t2r_rf_boardCfg;

STRU_cfgNode EK_2t2r_rfcfg_nodeInfo =
{
    .nodeId       = RF_BOARDCFG_PARA_ID,
    .nodeElemCnt  = 1,
    .nodeDataSize = sizeof(stru_EK_2t2r_rf_boardCfg)
};


STRU_BOARD_RF_PARA stru_EK_2t2r_rf_boardCfg __attribute__ ((aligned (4)))= 
{
    .u8_rfCnt                    = 1,

    .u8_rf0SkyRegsCnt            = 0,
    
    .u8_rf0GrdRegsCnt            = 0,
    
    .u8_rf1GrdRegsCnt            = 0,

     //after calibration

    .u8_rf0SkyRegsCntAfterCali   = 4,

    .u8_rf0GrdRegsCntAfterCali   = 7,
    
    .u8_rf0GrdRegsCntAfterCali   = 0,

    .boardCfgName                = "EK BBRF_2T2R",
};



extern STRU_RF_REG EK_2t2r_rf_boardReg[11];

STRU_cfgNode EK_2t2r_nodeInfo =
{
    .nodeId       = RF_BOARDCFG_DATA_ID,
    .nodeElemCnt  = 11,
    .nodeDataSize = sizeof(EK_2t2r_rf_boardReg)
};


STRU_RF_REG EK_2t2r_rf_boardReg[11] __attribute__ ((aligned (4)))=
{
    //pstru_rf1SkyRegs

    //pstru_rf2SkyRegs

    //pstru_rf1GrdRegs

    //pstru_rf2GrdRegs

    //ek2T2R_rf1_skyregs_afterCali
    {0, 0x35, 0x70},
    {0, 0x45, 0x87},
    {0, 0x66, 0x06},   //5G RF power
    {0, 0x67, 0x06},   //5G RF power

    //ek2T2R_rf1_grdregs_afterCali
    {0, 0x35, 0x70},
    {0, 0x45, 0x87},
    {0, 0x00, 0x74},   //1Tx only
    {0, 0x2D, 0xF6},
    {0, 0x37, 0xE0},
    {0, 0x66, 0x06},   //5G RF power
    {0, 0x67, 0x06},   //5G RF power
};
