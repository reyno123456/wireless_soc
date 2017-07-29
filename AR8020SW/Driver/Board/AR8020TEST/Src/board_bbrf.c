#include <string.h>
#include "boardParameters.h"


const STRU_BB_REG AR8020TEST_bb_skyregsAfterCali[] = 
{
    {1, 0x90, 0xFF}, //for 2TX
    {1, 0x91, 0x78}, //2RX
};


const STRU_BB_REG AR8020TEST_bb_grdregsAfterCali[] = 
{
    {1, 0x90, 0xF7}, //turn off DAC_B for 1TX
    {1, 0x91, 0x78}, //2RX
};


const STRU_RF_REG AR8020TEST_rf1_grdregs_afterCali[] = 
{
    {0x35, 0x70},
    {0x45, 0x87},
    {0x00, 0x74}, //1Tx only
    {0x2D, 0xF6},
    {0x37, 0xE0}
};

const STRU_RF_REG AR8020TEST_rf1_skyregs_afterCali[] = 
{
    {0x35, 0x70},
    {0x45, 0x87},
};



STRU_BoardCfg stru_boardCfg = 
{
    .u8_bbSkyRegsCnt 	= 0,
    .pstru_bbSkyRegs    = NULL,
    
    .u8_bbGrdRegsCnt    = 0,
    .pstru_bbGrdRegs    = NULL,

    .u8_rfCnt 	    = 1,

    .u8_rf1SkyRegsCnt   = 0,
    .pstru_rf1SkyRegs   = NULL,
    
    .u8_rf1GrdRegsCnt   = 0,
    .pstru_rf1GrdRegs   = NULL,
    
    .u8_rf2GrdRegsCnt   = 0,
    .pstru_rf2GrdRegs   = NULL,

    .e_bandsupport      = RF_2G,
     //after calibration
    .u8_bbSkyRegsCntAfterCali 	 = sizeof(AR8020TEST_bb_skyregsAfterCali) / sizeof(STRU_BB_REG),
    .pstru_bbSkyRegsAfterCali    = AR8020TEST_bb_skyregsAfterCali,

    .u8_bbGrdRegsCntAfterCali    = sizeof(AR8020TEST_bb_grdregsAfterCali) / sizeof(STRU_BB_REG),
    .pstru_bbGrdRegsAfterCali    = AR8020TEST_bb_grdregsAfterCali,

    .u8_rf1SkyRegsCntAfterCali   = sizeof(AR8020TEST_rf1_skyregs_afterCali) / sizeof(STRU_RF_REG),
    .pstru_rf1SkyRegsAfterCali   = AR8020TEST_rf1_skyregs_afterCali,

    .u8_rf1GrdRegsCntAfterCali   = sizeof(AR8020TEST_rf1_grdregs_afterCali) / sizeof(STRU_RF_REG),
    .pstru_rf1GrdRegsAfterCali   = AR8020TEST_rf1_grdregs_afterCali,
    
    .u8_rf2GrdRegsCntAfterCali   = 0,
    .pstru_rf2GrdRegsAfterCali   = NULL,

    .u8_bbStartMcs10M               = 0,
    .u8_bbStartMcs20M               = 0,
};

