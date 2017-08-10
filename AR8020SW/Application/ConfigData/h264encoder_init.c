#include <stdint.h>

#include "h264_encoder.h"
#include "cfg_parser.h"


extern STRU_H264_REG h264_init_reg[0];

STRU_cfgNode vsoc_enc_nodeInfo =
{
    .nodeId       = VSOC_ENC_INIT_ID,
    .nodeElemCnt  = 0,
    .nodeDataSize = sizeof(h264_init_reg)
};


STRU_H264_REG h264_init_reg[0] __attribute__ ((aligned (4)))= 
{

};
