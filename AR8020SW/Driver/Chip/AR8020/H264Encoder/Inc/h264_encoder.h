#ifndef H264_ENCODER_H
#define H264_ENCODER_H

#include <stdint.h>

typedef struct
{
    uint32_t u32_regAddr;
    uint32_t u32_regValue;
    uint32_t u32_regdataMask;
}STRU_H264_REG;
int H264_Encoder_Init(uint8_t gop0, uint8_t br0, uint8_t brc0_e, uint8_t gop1, uint8_t br1, uint8_t brc1_e);
int H264_Encoder_UpdateBitrate(unsigned char view, unsigned char br);
void H264_Encoder_DumpFrameCount(void);

#endif

