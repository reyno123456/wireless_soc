#include <string.h>
#include "boardParameters.h"
#include "cfg_parser.h"
#include "bb_types.h"


////////////////////////////////////////////////
extern STRU_BOARD_BB_PARA stru_TEST8003_bb_boardCfg;

STRU_cfgNode TEST8003_bbcfg_nodeInfo= 
{
    .nodeId       = BB_BOARDCFG_PARA_ID,
    .nodeElemCnt  = 1,    
    .nodeDataSize = sizeof(stru_TEST8003_bb_boardCfg)
};


STRU_BOARD_BB_PARA stru_TEST8003_bb_boardCfg __attribute__ ((aligned (4)))= 
{
    .u8_bbSkyRegsCnt             = 0,
    
    .u8_bbGrdRegsCnt             = 0,

    .e_bandsupport               = RF_2G,

     //after calibration
    .u8_bbSkyRegsCntAfterCali    = 2,

    .u8_bbGrdRegsCntAfterCali    = 2,

    .u8_bbStartMcs10M            = 0,
    .u8_bbStartMcs20M            = 0,
};


extern STRU_BB_REG stru_TEST8003_bb_reg[6];

STRU_cfgNode TEST8003_bbdata_nodeInfo =
{
    .nodeId       = BB_BOARDCFG_DATA_ID,
    .nodeElemCnt  = 6,
    .nodeDataSize = sizeof(stru_TEST8003_bb_reg)
};


STRU_BB_REG stru_TEST8003_bb_reg[6] __attribute__ ((aligned (4)))= 
{
    //ek2T2R_bb_skyregsAfterCali
    {1, 0x90, 0xFF}, //for 2TX
    {1, 0x91, 0x78}, //2RX
    
    //ek2T2R_bb_grdregsAfterCali
    {1, 0x90, 0xF7}, //turn off DAC_B for 1TX
    {1, 0x91, 0x78}, //2RX
};



////////////////////////////////////////////////

extern STRU_BOARD_RF_PARA stru_TEST8003_rf_boardCfg;

STRU_cfgNode TEST8003_rfcfg_nodeInfo =
{
    .nodeId       = RF_BOARDCFG_PARA_ID,
    .nodeElemCnt  = 1,
    .nodeDataSize = sizeof(stru_TEST8003_rf_boardCfg)
};


STRU_BOARD_RF_PARA stru_TEST8003_rf_boardCfg __attribute__ ((aligned (4)))= 
{
    .u8_rfCnt                    = 1,

    .u8_rf0SkyRegsCnt            = 0,
    
    .u8_rf0GrdRegsCnt            = 0,
    
    .u8_rf1GrdRegsCnt            = 0,

     //after calibration

    .u8_rf0SkyRegsCntAfterCali   = 1,

    .u8_rf0GrdRegsCntAfterCali   = 5,
    
    .u8_rf0GrdRegsCntAfterCali   = 0,

    .boardCfgName                = "TEST8003_2T2R",
};



extern STRU_RF_REG TEST8003_rf_boardReg[6];

STRU_cfgNode TEST8003_nodeInfo =
{
    .nodeId       = RF_BOARDCFG_DATA_ID,
    .nodeElemCnt  = 6,
    .nodeDataSize = sizeof(TEST8003_rf_boardReg)
};


STRU_RF_REG TEST8003_rf_boardReg[6] __attribute__ ((aligned (4)))=
{
    //pstru_rf1SkyRegs

    //pstru_rf2SkyRegs

    //pstru_rf1GrdRegs

    //pstru_rf2GrdRegs

    //ek2T2R_rf1_skyregs_afterCali
    {0, 0x35, 0x70},

    //ek2T2R_rf1_grdregs_afterCali
    {0, 0x35, 0x70},
    {0, 0x45, 0x87},
    {0, 0x00, 0x74},   //1Tx only
    {0, 0x2D, 0xF6},
    {0, 0x37, 0xE0},
};
