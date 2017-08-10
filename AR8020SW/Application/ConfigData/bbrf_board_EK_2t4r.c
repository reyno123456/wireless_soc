#include <string.h>
#include "boardParameters.h"

#include "cfg_parser.h"
#include "bb_types.h"


////////////////////////////////////////////////
extern STRU_BOARD_BB_PARA stru_EK_2t4r_bb_boardCfg;

STRU_cfgNode AR8020EK_nodeInfo= 
{
    .nodeId       = BB_BOARDCFG_PARA_ID,
    .nodeElemCnt  = 1,
    .nodeDataSize = sizeof(stru_EK_2t4r_bb_boardCfg),
};

STRU_BOARD_BB_PARA stru_EK_2t4r_bb_boardCfg __attribute__ ((aligned (4)))= 
{
    .u8_bbSkyRegsCnt    = 0,
    
    .u8_bbGrdRegsCnt    = 1,

    .e_bandsupport      = RF_2G_5G,
     //after calibration
    .u8_bbSkyRegsCntAfterCali    = 3,

    .u8_bbGrdRegsCntAfterCali    = 2,

    .u8_bbStartMcs10M            = 0,
    .u8_bbStartMcs20M            = 0, 
};


extern STRU_BB_REG stru_EK_2t4r_bb_reg[6];

STRU_cfgNode EK_2t4r_bbdata_nodeInfo =
{
    .nodeId       = BB_BOARDCFG_DATA_ID,
    .nodeElemCnt  = 6,
    .nodeDataSize = sizeof(stru_EK_2t4r_bb_reg)
};


STRU_BB_REG stru_EK_2t4r_bb_reg[6] __attribute__ ((aligned (4)))= 
{
    //pstru_bbSkyRegs

    //pstru_bbGrdRegs
    {2, 0x08, 0x72}  //2T4R

    //AR8020TEST_bb_skyregsAfterCali
    {1, 0x90, 0xFF}, //2TX
    {1, 0x91, 0x78}, //2RX
    {2, 0x03, 0x80}  //2T


    //ek2t4r_bb_grdregsAfterCali
    {1, 0x90, 0xF7}, //turn off DAC_B for 1TX
    {1, 0x91, 0xFF}, //4RX

};



////////////////////////////////////////////////

extern STRU_BOARD_RF_PARA stru_EK_2t4r_rf_boardCfg;

STRU_cfgNode EK_2t4r_rfcfg_nodeInfo =
{
    .nodeId       = RF_BOARDCFG_PARA_ID,
    .nodeElemCnt  = 1,
    .nodeDataSize = sizeof(stru_EK_2t4r_rf_boardCfg)
};


STRU_BOARD_RF_PARA stru_EK_2t4r_rf_boardCfg __attribute__ ((aligned (4)))= 
{
    .u8_rfCnt                    = 2,

    .u8_rf0SkyRegsCnt            = 0,
    
    .u8_rf0GrdRegsCnt            = 1,
    
    .u8_rf1GrdRegsCnt            = 0,

     //after calibration

    .u8_rf0SkyRegsCntAfterCali   = 2,

    .u8_rf0GrdRegsCntAfterCali   = 5,
    
    .u8_rf0GrdRegsCntAfterCali   = 6,

    .boardCfgName                = "EK BBRF_2T4R",
};




extern STRU_RF_REG EK_2t4r_rf_boardReg[14];

STRU_cfgNode stru_EK_2t4r_nodeInfo =
{
    .nodeId       = RF_BOARDCFG_DATA_ID,
    .nodeElemCnt  = 14,
    .nodeDataSize = sizeof(EK_2t4r_rf_boardReg)
};



STRU_RF_REG EK_2t4r_rf_boardReg[14] __attribute__ ((aligned (4)))=
{
    //pstru_rf1SkyRegs

    //pstru_rf2SkyRegs

    //pstru_rf1GrdRegs
    {0, 0x71, 0x04},   //40M clock for RF8003S_2

    //pstru_rf2GrdRegs

    //ek2t4r_rf1_skyregs_afterCali
    {0, 0x35, 0x70},
    {0, 0x45, 0x87},

    //ek2t4r_rf1_grdregs_afterCali
    {0, 0x35, 0x70},
    {0, 0x45, 0x87},
    {0, 0x00, 0x74}, //1Tx only
    {0, 0x2D, 0xF6},
    {0, 0x37, 0xE0}

    //ek2T4R_rf2_grdregs_afterCali
    {0, 0x35, 0x70},
    {0, 0x45, 0x87},
    {0, 0x00, 0x60}, //RX only for 2G
    {0, 0x40, 0x60}, //RX only for 5G
    {0, 0x2D, 0xF6},
    {0, 0x37, 0xE0},
};
