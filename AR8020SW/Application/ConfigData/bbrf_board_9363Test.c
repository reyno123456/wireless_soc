#include <string.h>
#include "boardParameters.h"

#include "cfg_parser.h"
#include "bb_types.h"


/////////////////////////////////////////////

extern STRU_BOARD_BB_PARA stru_9363Test_bb_boardCfg;

STRU_cfgNode AR8020_9363Test_bbcfg_nodeInfo= 
{
    .nodeId       = BB_BOARDCFG_PARA_ID,
    .nodeElemCnt  = 1,
    .nodeDataSize = sizeof(stru_9363Test_bb_boardCfg)
};

STRU_BOARD_BB_PARA stru_9363Test_bb_boardCfg = 
{
    .u8_bbSkyRegsCnt    = 45,
    
    .u8_bbGrdRegsCnt    = 51,

    .e_bandsupport      = RF_600M,

     //after calibration
    .u8_bbSkyRegsCntAfterCali    = 2,

    .u8_bbGrdRegsCntAfterCali    = 2,

    .u8_bbStartMcs10M            = 0,
    .u8_bbStartMcs20M            = 0,
};



/////////////////////////////////////////////

extern STRU_BB_REG Test9363_bb_regCfg[100];

STRU_cfgNode Test9363_nodeInfo =
{
    .nodeId       = BB_BOARDCFG_DATA_ID,
    .nodeElemCnt  = 100,
    .nodeDataSize = sizeof(Test9363_nodeInfo)
};


STRU_BB_REG Test9363_bb_regCfg[100] __attribute__ ((aligned (4)))= 
{
    //AR8020TEST9363_bb_skyregs
    {0, 0x90, 0x34},
    {0, 0x91, 0x40},
    {0, 0x92, 0x30},
    {0, 0x93, 0x30},
    {0, 0x96, 0x28},
    {0, 0x97, 0xc0},
    {0, 0x98, 0x14},
    {0, 0x9c, 0x1c},
    {0, 0x9d, 0x30},

    {0, 0xa0, 0x19},
    {0, 0xa1, 0xc4},    
    {0, 0xa2, 0x00},
    {0, 0x9d, 0x00},
    {0, 0x9e, 0x00},
    {0, 0x9f, 0x00},

    {0, 0xb0, 0x80},
    {0, 0xb1, 0x08},    
    {0, 0xb3, 0x80},
    {0, 0xb4, 0x08},
    {0, 0xbc, 0x00},
    {0, 0xbd, 0x00},    
    {0, 0xbe, 0x00},
    {0, 0xbf, 0x00},

    {0, 0xc6, 0x00},
    {0, 0xc7, 0x00},
    {0, 0xc8, 0x00},
    {0, 0xc9, 0x00},
    {0, 0xca, 0x00},
    {0, 0xcd, 0x00},
    {0, 0xce, 0x00},
    {0, 0xcf, 0x00},
    
    {2, 0x10, 0x8c},
    {2, 0x13, 0x00}, 
    {2, 0x1a, 0x8c},
    {2, 0x1b, 0x00},  
    {2, 0x1c, 0x00},
    {2, 0x1d, 0x00},  
    {2, 0x1f, 0x20},
    {2, 0x20, 0x00},
    {2, 0x21, 0x20},
    {2, 0x2d, 0x00},
    {2, 0x2e, 0x00},
    {2, 0x2f, 0x00},
	{0, 0x09, 0x3F}, // baseband TX power 
    {0, 0x16, 0x3F},

    //AR8020TEST9363_bb_groundregs
    {0, 0x90, 0x38},
    {0, 0x91, 0x40},
    {0, 0x92, 0x20},
    {0, 0x93, 0x20},
    {0, 0x97, 0x08},
    {0, 0x98, 0x14},
    {0, 0x9d, 0x0c},

    {0, 0xa0, 0x38},
    {0, 0xa2, 0xc0}, 
    {0, 0xa3, 0xa8},
    {0, 0xa4, 0xd9},
    {0, 0xa5, 0x8a},
    {0, 0xa6, 0x73},
    
    {0, 0xa7, 0x65},
    {0, 0xa8, 0x7c},
    {0, 0xa9, 0x70},
    {0, 0xaa, 0x60},
    {0, 0xab, 0x50},
    {0, 0xac, 0x48},

    {0, 0xb0, 0x00},
    {0, 0xb1, 0x00}, 
    {0, 0xb2, 0x00}, 
    {0, 0xb3, 0x00},
    {0, 0xb4, 0x00},
    {0, 0xb5, 0x00},
    {0, 0xb6, 0x00}, 
    {0, 0xb7, 0x00},
    {0, 0xb8, 0x00},
    {0, 0xb9, 0x00},
    {0, 0xba, 0x00},
    {0, 0xbb, 0x00},
    {0, 0xbc, 0x00},
    {0, 0xbe, 0x00},

    {0, 0xc0, 0x00},
    {0, 0xc1, 0x00},
    {0, 0xc6, 0x00},
    {0, 0xc7, 0x00},
    {0, 0xca, 0x00},
    {0, 0xce, 0x00},
    
    {2, 0x10, 0x8c},
    {2, 0x13, 0x00}, 
    {2, 0x1a, 0x8c},
    {2, 0x1b, 0x00}, 
    {2, 0x1c, 0x00},
    {2, 0x1d, 0x00}, 
    {2, 0x1f, 0x20},
    {2, 0x20, 0x00},
    {2, 0x21, 0x20},
    {2, 0x2f, 0x00},
	{0, 0x09, 0x3F}, // baseband TX power 
    {0, 0x16, 0x3F},

    //AR8020TEST9363_bb_skyregsAfterCali
    {1, 0x90, 0xFF}, //for 2TX
    {1, 0x91, 0x78}, //2RX

    //AR8020TEST9363_bb_grdregsAfterCali
    {1, 0x90, 0xF7}, //turn off DAC_B for 1TX
    {1, 0x91, 0x78}, //2RX
};



////////////////////////////////////////////////////////////
extern STRU_BOARD_RF_PARA stru_9363Test_rf_boardCfg;

STRU_cfgNode AR8020_9363Test_nodeInfo= 
{
    .nodeId       = RF_BOARDCFG_PARA_ID,
    .nodeElemCnt  = 1,
    .nodeDataSize = sizeof(stru_9363Test_bb_boardCfg)
};

STRU_BOARD_RF_PARA stru_9363Test_rf_boardCfg = 
{
    .u8_rfCnt           = 1,

    .u8_rf0SkyRegsCnt   = 0,
    
    .u8_rf0GrdRegsCnt   = 0,
    
    .u8_rf1GrdRegsCnt   = 0,

     //after calibration
    .u8_rf0SkyRegsCntAfterCali   = 0,

    .u8_rf0GrdRegsCntAfterCali   = 0,
    
    .u8_rf0GrdRegsCntAfterCali   = 0,

    .boardCfgName                = "AR9363Test"
};



extern STRU_RF_REG stru_9363Test_rf_regCfg[0];

STRU_cfgNode stru_9363Test_bbrfdata_nodeInfo =
{
    .nodeId       = RF_BOARDCFG_DATA_ID,
    .nodeElemCnt  = 0,
    .nodeDataSize = sizeof(stru_9363Test_rf_regCfg)
};

STRU_RF_REG stru_9363Test_rf_regCfg[0] __attribute__ ((aligned (4)))= 
{
    //pstru_rf1SkyRegs
    //pstru_rf2SkyRegs
    //pstru_rf1GrdRegs
    //pstru_rf2GrdRegs
    
    //rf1_skyregs_afterCali

    //rf1_grdregs_afterCali 
};

