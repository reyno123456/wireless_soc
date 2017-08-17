#ifndef _CONFIG_BIN_H_
#define _CONFIG_BIN_H_

#include <stdint.h>


//HDMI
#define HDMI_EDID_CFG_ID                                (0)


//Baseband
#define BB_SKY_REG_INIT_NODE_ID                         (1)
#define BB_GRD_REG_INIT_NODE_ID                         (2)


#define RF_INIT_REG_NODE_ID_0                           (3)
#define RF_INIT_REG_NODE_ID_1                           (4)


#define RF_SKY_INIT_REG_ID_0                            (5)
#define RF_GRD_INIT_REG_ID_0                            (6)


#define RF_RC_BAND0_10M_FRQ_ID                          (7)
#define RF_IT_BAND0_10M_FRQ_ID                          (8)


#define RF_RC_BAND0_20M_FRQ_ID                          (9)
#define RF_IT_BAND0_20M_FRQ_ID                          (10)


#define RF_SWEEP_BAND0_20M_FRQ_ID                       (11)


#define RF_BAND1_10M_FRQ_ID                             (12)
#define RF_IT_BAND1_10M_FRQ_ID                          (13)


#define RF_RC_BAND1_20M_FRQ_ID                          (14)
#define RF_IT_BAND1_20M_FRQ_ID                          (15)

#define RF_SWEEP_BAND1_20M_FRQ_ID                       (16)


//board config
#define BB_BOARDCFG_PARA_ID                             (17)
#define BB_BOARDCFG_DATA_ID                             (18)

#define RF_BOARDCFG_PARA_ID                             (19)
#define RF_BOARDCFG_DATA_ID                             (20)


#define VSOC_ENC_INIT_ID                                (21)


#define CFG_DATA_HEAD_FLAG                              (0x3553F708)


typedef struct
{
    uint32_t nodeId;
    uint32_t nodeElemCnt;
    uint32_t nodeDataSize;
}STRU_cfgNode;


typedef struct
{
    uint32_t headflag;      //
    uint32_t version;       //
    uint32_t dataSize;      //sum of the elemSizes
    uint8_t  dataMd5[16];   //
}STRU_cfgBin;


void CFGBIN_LoadFromFlash(uint32_t dstAddr, uint32_t srcAddr);

STRU_cfgNode *CFGBIN_GetNode(STRU_cfgBin *ptr_cfg, uint32_t u32_nodeId);


#endif // _CONFIG_BIN_H_
