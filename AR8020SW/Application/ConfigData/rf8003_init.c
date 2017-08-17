#include <stdint.h>
#include "cfg_parser.h"


extern uint8_t RF_8003_regs0[64];

STRU_cfgNode RF8803s_0_nodeInfo = 
{
    .nodeId       = RF_INIT_REG_NODE_ID_0,
    .nodeElemCnt  = 64,
    .nodeDataSize = sizeof(RF_8003_regs0)
};

uint8_t RF_8003_regs0[64] __attribute__ ((aligned (4))) = 
{
    0x7C,0x00,0x00,0xC0,0x00,0x02,0x00,0x00,0xF3,0x00,0x00,0x00,0xF3,0x00,0x00,0x00,
    0x00,0x80,0x80,0x74,0x60,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4E,0x00,0x00,
    0x00,0x40,0x40,0x48,0x00,0x54,0x00,0x00,0x00,0x00,0x10,0x00,0x1F,0x00,0x10,0x0F,
    0x1F,0x00,0x28,0x40,0x00,0x70,0xE0,0x00,0x37,0x00,0x14,0x00,0x3F,0x3D,0x7F,0xFF,
};



extern uint8_t RF8003_regs1[64];

STRU_cfgNode RF8803s_1_nodeInfo =
{
    .nodeId       = RF_INIT_REG_NODE_ID_1,
    .nodeElemCnt  = 64,
    .nodeDataSize = sizeof(RF8003_regs1)
};

uint8_t RF8003_regs1[64]  __attribute__ ((aligned (4))) = 
{
    0x7C,0x00,0x00,0xC0,0x00,0x02,0x00,0x00,0xF3,0x00,0x00,0x00,0xF3,0x00,0x00,0x00,
    0x00,0x80,0x80,0x74,0x60,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4E,0x00,0x00,
    0x00,0x40,0x40,0x48,0x00,0x54,0x00,0x00,0x00,0x00,0x10,0x00,0x1F,0x00,0x10,0x0F,
    0x1F,0x00,0x28,0x40,0x00,0x70,0xE0,0x00,0x37,0x00,0x14,0x00,0x3F,0x3D,0x7F,0xFF,
};



extern uint8_t RF8003_RC_2_4G_frq[34][5];

STRU_cfgNode RF8003_Rc_2_4G_nodeInfo = 
{
    .nodeId       = RF_RC_BAND0_10M_FRQ_ID,
    .nodeElemCnt  = 34,
    .nodeDataSize = sizeof(RF8003_RC_2_4G_frq)
};

uint8_t RF8003_RC_2_4G_frq[34][5] __attribute__ ((aligned (4))) =    // 2.4G
{
    {0x34, 0x48, 0x83, 0x7C, 0x00},     //2428
    {0xD8, 0x89, 0x9D, 0x7C, 0x00},     //2430
    {0xB7, 0x7C, 0xCB, 0x7F, 0x00},     //2492
    {0x5B, 0xBE, 0xE5, 0x7F, 0x00},     //2494
    {0x7C, 0xCB, 0xB7, 0x7C, 0x00},     //2432
    {0x20, 0x0D, 0xD2, 0x7C, 0x00},     //2434
    {0x6F, 0xF9, 0x96, 0x7F, 0x00},     //2488
    {0x13, 0x3B, 0xB1, 0x7F, 0x00},     //2490
    {0xC4, 0x4E, 0xEC, 0x7C, 0x00},     //2436
    {0x69, 0x90, 0x06, 0x7D, 0x00},     //2438
    {0x0D, 0xD2, 0x20, 0x7D, 0x00},     //2440
    {0xB1, 0x13, 0x3B, 0x7D, 0x00},     //2442
    {0x27, 0x76, 0x62, 0x7F, 0x00},     //2484
    {0xCB, 0xB7, 0x7C, 0x7F, 0x00},     //2486
    {0x55, 0x55, 0x55, 0x7D, 0x00},     //2444
    {0xF9, 0x96, 0x6F, 0x7D, 0x00},     //2446
    {0xDF, 0xF2, 0x2D, 0x7F, 0x00},     //2480
    {0x83, 0x34, 0x48, 0x7F, 0x00},     //2482
    {0x9D, 0xD8, 0x89, 0x7D, 0x00},     //2448
    {0x41, 0x1A, 0xA4, 0x7D, 0x00},     //2450
    {0xE5, 0x5B, 0xBE, 0x7D, 0x00},     //2452
    {0x89, 0x9D, 0xD8, 0x7D, 0x00},     //2454
    {0x96, 0x6F, 0xF9, 0x7E, 0x00},     //2476
    {0x3B, 0xB1, 0x13, 0x7F, 0x00},     //2478
    {0x2D, 0xDF, 0xF2, 0x7D, 0x00},     //2456
    {0xD2, 0x20, 0x0D, 0x7E, 0x00},     //2458
    {0x76, 0x62, 0x27, 0x7E, 0x00},     //2460
    {0x1A, 0xA4, 0x41, 0x7E, 0x00},     //2462
    {0x4E, 0xEC, 0xC4, 0x7E, 0x00},     //2472
    {0xF2, 0x2D, 0xDF, 0x7E, 0x00},     //2474
    {0xBE, 0xE5, 0x5B, 0x7E, 0x00},     //2464
    {0x62, 0x27, 0x76, 0x7E, 0x00},     //2466
    {0x06, 0x69, 0x90, 0x7E, 0x00},     //2468
    {0xAA, 0xAA, 0xAA, 0x7E, 0x00},     //2470
};



extern uint8_t RF8003_It_2_4G_frq[8][5];

STRU_cfgNode RF8803s_It_2_4G_nodeInfo = {
    .nodeId       = RF_IT_BAND0_10M_FRQ_ID,
    .nodeElemCnt  = 8,
    .nodeDataSize = sizeof(RF8003_It_2_4G_frq)
};

uint8_t RF8003_It_2_4G_frq[8][5] __attribute__ ((aligned (4))) =  //2.4G
{
    {0x27, 0x76, 0x62, 0x7B, 0x00},   //2406
    {0x5B, 0xBE, 0xE5, 0x7B, 0x00},   //2416
    {0x90, 0x06, 0x69, 0x7C, 0x00},   //2426
    {0xC4, 0x4E, 0xEC, 0x7C, 0x00},   //2436
    {0xF9, 0x96, 0x6F, 0x7D, 0x00},   //2446
    {0x2D, 0xDF, 0xF2, 0x7D, 0x00},   //2456
    {0x62, 0x27, 0x76, 0x7E, 0x00},   //2466
    {0x96, 0x6F, 0xF9, 0x7E, 0x00},   //2476
};


extern uint8_t RF8003_2_4G_20M_sweep_frq[4][5];

STRU_cfgNode RF8003_2_4G_sweep_20M_nodeInfo = 
{
    .nodeId       = RF_SWEEP_BAND0_20M_FRQ_ID,
    .nodeElemCnt  = 4,
    .nodeDataSize = sizeof(RF8003_2_4G_20M_sweep_frq)
};


uint8_t RF8003_2_4G_20M_sweep_frq[4][5]  __attribute__ ((aligned (4))) =
{
    {0x41, 0x1A, 0xA4, 0x7B, 0x00},    //2411
    {0xAA, 0xAA, 0xAA, 0x7C, 0x00},    //2431
    {0x13, 0x3B, 0xB1, 0x7D, 0x00},    //2451
    {0x7C, 0xCB, 0xB7, 0x7E, 0x00},    //2471
};
