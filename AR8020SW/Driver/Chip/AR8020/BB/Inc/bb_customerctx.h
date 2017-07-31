#ifndef __BB_CUSTOMER_CTX_H_
#define __BB_CUSTOMER_CTX_H_


#include "boardParameters.h"


typedef struct
{
    uint8_t     flag_useCfgId;
    uint8_t     rcid[5];

    ENUM_CH_BW  enum_chBandWidth;

    STRU_BoardCfg *pstru_boardCfg;

} STRU_CUSTOMER_CFG;


#endif
