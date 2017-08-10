#include <stdint.h>
#include "cfg_parser.h"

extern uint8_t BB_sky_regs[4][256];
STRU_cfgNode BB_sky_nodeInfo = 
{
    .nodeId       = BB_SKY_REG_INIT_NODE_ID,
    .nodeElemCnt  = 1024,
    .nodeDataSize = sizeof(BB_sky_regs)
};

uint8_t BB_sky_regs[4][256] __attribute__ ((aligned (4))) = 
{
    {
        0x00, 0x02, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x18, 0x14, 0x01, 0x86, 0xF0,
        0x01, 0x01, 0x30, 0x01, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E,
        0xFE, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0xAA, 0x02, 0x00, 0x00, 0xC0,
        0x7E, 0x96, 0x02, 0x72, 0x20, 0x1B, 0x1A, 0xDB, 0x1A, 0xDB, 0x02, 0x72, 0x01, 0x58, 0x02, 0x72,
        0x00, 0x00, 0x00, 0x4C, 0x6D, 0x00, 0x4C, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x0C, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x50, 0x01, 0x00, 0x34, 0xA6, 0xA6, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x24, 0x00, 0x10, 0xC7, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0E, 0x02, 0x01, 0x00, 0x09, 0x01, 0x00,
        0x3D, 0x71, 0x12, 0x12, 0x0F, 0x3F, 0x18, 0x3F, 0x40, 0x00, 0x13, 0x12, 0x13, 0x00, 0x00, 0x00, //0x70: LNA on   //0x70->0x71 5G RC unlock->lock
        0x30, 0xC4, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 0x96, 0x80, //[0xa0]-> 0x30 For sweep
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11, 0x0a, 0x03,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x12, 0xF0, 0xB0, 0x30, 0x00, 0x00, 0x44, 0x0a, 0x50,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD6, 0x02, 0x01, 0x00, 0x09, 0x10, 0x00,
    },
    {
        0x40, 0x02, 0x24, 0x40, 0x00, 0x00, 0x00, 0x00, 0x53, 0x96, 0xD9, 0xFF, 0x04, 0x00, 0xFF, 0xFF,
        0x69, 0x51, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x72, 0x0A, 0x05, 0x09, 0x01, 0x14, 0x80, 0x00, 0x00, 0x00, 0x00, 0x14, 0x50, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x02, 0x0D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x40, 0x29,
        0x10, 0x00, 0x00, 0x00, 0x04, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xC3, 0x00, 0x00, 0x84, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x07, 0x00, 0x00, 0x00, 0x64, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00,
        0xFF, 0xFF, 0x00, 0x00, 0x00, 0x1C, 0x1C, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00,
        0x00, 0xFA, 0x00, 0x00, 0xA6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x00, 0x00,
        0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xF0, 0x00, 0xFF, 0x9D, 0x04, 0xFA, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4D, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {
        0x80, 0x02, 0x07, 0x80, 0x90, 0xEF, 0x44, 0x09, 0x17, 0x80, 0xC0, 0x2D, 0x00, 0x40, 0xFE, 0x47,
        0x00, 0x00, 0x00, 0x4E, 0x00, 0x00, 0x00, 0x4E, 0x00, 0x00, 0x00, 0x4E, 0xF0, 0xB0, 0x00, 0x00,
        0xC0, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x4C,
        0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0x60, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFA, 0x00,
        0x89, 0xAB, 0x6C, 0xF5, 0x6E, 0x03, 0x54, 0x70, 0xE3, 0x52, 0x9C, 0x4A, 0x9C, 0xA2, 0xE0, 0x07,
        0x13, 0xFF, 0x0B, 0xA6, 0xF0, 0x88, 0x6C, 0x85, 0x8D, 0x52, 0xEC, 0xE7, 0xCA, 0xC6, 0x3E, 0x2C,
        0xF1, 0xDB, 0x2F, 0xAC, 0x5B, 0x7C, 0xF3, 0x4E, 0x78, 0x37, 0x63, 0xC6, 0x0A, 0xAB, 0x4A, 0xAC,
        0x15, 0x34, 0xB4, 0x35, 0x70, 0x03, 0x2D, 0x3D, 0x2F, 0x96, 0xB6, 0x27, 0x89, 0xD7, 0x82, 0x8D,
        0x00, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x10, 0x10, 0x16, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xC0, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    {
        0xC0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x05, 0x00, 0x04, 0x00, 0x06, 0x00, 0x0B, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x04,
        0x00, 0x06, 0x00, 0x0D, 0x00, 0x07, 0x00, 0x09, 0x00, 0x1B, 0x00, 0x35, 0x00, 0x2B, 0x00, 0x31,
        0x00, 0x39, 0x00, 0x31, 0x00, 0x2F, 0x00, 0x1F, 0x00, 0x07, 0x00, 0x05, 0x00, 0x0A, 0x00, 0x04,
        0x00, 0x06, 0x00, 0x06, 0x00, 0x04, 0x00, 0x06, 0x00, 0x0B, 0x00, 0x05, 0x00, 0x06, 0x00, 0x06,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x24, 0x00, 0x3E, 0x1F, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    }
};


extern uint8_t BB_grd_regs[4][256];
STRU_cfgNode BB_grd_nodeInfo = 
{
    .nodeId       = BB_GRD_REG_INIT_NODE_ID,
    .nodeElemCnt  = 1024,
    .nodeDataSize = sizeof(BB_grd_regs)
};

uint8_t BB_grd_regs[4][256]  __attribute__ ((aligned (4))) = 
{
    {
        0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x40, 0x00, 0x01, 0x86, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26,
        0xFE, 0x0A, 0x1B, 0x77, 0x02, 0x71, 0x80, 0x6B, 0x7D, 0xFA, 0x7D, 0xFA, 0x02, 0x71, 0x01, 0xB7,
        0x20, 0x00, 0x00, 0x60, 0x04, 0xE2, 0x08, 0x8C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x02, 0x64, 0x60, 0x02, 0x64, 0x80, 0x04, 0xC9, 0x00, 0x04, 0xC9, 0x00, 0xFF, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x11, 0x74, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x50, 0x01, 0x01, 0x34, 0xA6, 0xA6, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x3D, 0x70, 0x2C, 0x25, 0x0F, 0x1F, 0x1F, 0x1F, 0x08, 0x15, 0x13, 0x12, 0x1C, 0x00, 0x00, 0x00, //0x70: LNA on
        0x30, 0xAC, 0xFF, 0xC0, 0x90, 0x80, 0x58, 0x50, 0xA0, 0x8B, 0x70, 0x48, 0x40, 0x00, 0x00, 0x00, //[0xa0]-> 0x30 For sweep
        0x1C, 0x22, 0x2A, 0x3A, 0xAF, 0xF1, 0x0F, 0x1F, 0x2F, 0x3E, 0xB5, 0xF6, 0x80, 0x00, 0x0a, 0x00,
        0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x25, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x0a, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x09, 0x10, 0x00,
    },
    {
        0x40, 0x02, 0x24, 0x20, 0x00, 0x00, 0x81, 0x01, 0x53, 0x96, 0xC9, 0x74, 0x08, 0x00, 0x00, 0x00,
        0x09, 0x61, 0x00, 0x00, 0x00, 0x00, 0x20, 0xD9, 0x04, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x0C, 0x06, 0xa2, 0x08, 0x02, 0x90, 0x52, 0x0A, 0x30, 0x0A, 0x28, 0x52, 0x00, 0x14, 0x53, 0x31, //0x7a->0xa2
        0x28, 0x91, 0x45, 0x0A, 0x30, 0x52, 0x02, 0x00, 0x20, 0x01, 0x00, 0x09, 0x01, 0x00, 0x10, 0x0A,
        0x0F, 0x0A, 0x64, 0x57, 0x77, 0x77, 0x72, 0x23, 0x44, 0x45, 0x57, 0x70, 0x87, 0x9F, 0xBF, 0xC9,
        0x00, 0x50, 0x31, 0x19, 0x88, 0x35, 0x05, 0x80, 0xD0, 0x34, 0x0B, 0xC0, 0x00, 0x01, 0x40, 0x40,
        0x30, 0x00, 0x00, 0x00, 0x07, 0x98, 0xFF, 0xFF, 0xFF, 0x4C, 0x71, 0x60, 0xFF, 0xFF, 0xFF, 0x29,
        0x00, 0x20, 0xFF, 0x37, 0x13, 0x84, 0x18, 0x45, 0x41, 0x01, 0x29, 0x70, 0x0A, 0x00, 0x30, 0x00,
        0xF0, 0xEF, 0x3D, 0x08, 0x10, 0x08, 0x00, 0x18, 0x5A, 0x00, 0x10, 0xA0, 0x08, 0xC1, 0x0F, 0x11, //0x40->0xF0. Reset number
        0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00,
        0x00, 0xFA, 0x00, 0x00, 0xA6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x00, 0x00,
        0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x73, 0x7D, 0x7D, 0x82, 0x02, 0x78, 0x8B, 0x0D, 0x04, 0x15, 0xF4, 0xF5, 0x01, 0xC0, 0xE8,
        0x00, 0x00, 0x02, 0x03, 0x04, 0x00, 0x20, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x0F, 0xE8, 0x1D, 0x06, 0x01, 0x92, 0x05,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    {
        //0x80, 0x02, 0x15, 0x00, 0x38, 0x0F, 0x00, 0x00, 0x62, 0x00, 0x06, 0xAB, 0x01, 0x14, 0x00, 0x06,
        0x80, 0x02, 0x15, 0x00, 0x38, 0x0F, 0x00, 0x00, 0x62, 0x00, 0x06, 0xAB, 0x01, 0x10, 0x00, 0x06,
        0x00, 0x00, 0x00, 0x4E, 0x00, 0x00, 0x00, 0x4E, 0x00, 0x00, 0x00, 0x4E, 0x00, 0x00, 0x00, 0x00,
        0xC0, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0xA8, 0x03, 0xA8, 0x03, 0xFF, 0xFF, 0xFF, 0x60, 0xFF,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x3F, 0x1D, 0x00, 0x00, 0xBB, 0x98, 0x94, 0x00, 0x00, 0x3A, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00,
        0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x12, 0x06, 0x00, 0xF2, 0x09, 0x11, 0x00,
        0x10, 0xEC, 0x05, 0xF7, 0x0D, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x0B,
        0x60, 0x01, 0x27, 0x00, 0xFF, 0xFF, 0xFA, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x06, 0x9F, 0x0E, 0xEE, 0x0F, 0xDE, 0x11, 0xE5, 0x11, 0x25, 0x0F, 0xC4, 0x0D, 0xBB, 0x0F, 0x1F,
        0x11, 0x8B, 0x0F, 0xBD, 0x0F, 0x26, 0x11, 0x64, 0x11, 0x71, 0x0E, 0x89, 0x11, 0x91, 0x11, 0x12,
    },
    {
        0xC0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x7D, 0x00, 0x00, 0x00, 0x7D, 0x00, 0x00, 0x00, 0x7D, 0x80, 0x2E, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x2D, 0x00, 0x2B, 0x00, 0x2D, 0x00, 0x3C, 0x00, 0x27, 0x00, 0x27, 0x00, 0x2E, 0x00, 0x24,
        0x00, 0x23, 0x00, 0x36, 0x00, 0x1E, 0x00, 0x1E, 0x00, 0x34, 0x00, 0x37, 0x00, 0x2B, 0x00, 0x41,
        0x00, 0x4A, 0x00, 0x2E, 0x00, 0x31, 0x00, 0x2E, 0x00, 0x24, 0x00, 0x23, 0x00, 0x2B, 0x00, 0x24,
        0x00, 0x29, 0x00, 0x2A, 0x00, 0x27, 0x00, 0x29, 0x00, 0x34, 0x00, 0x2D, 0x00, 0x2B, 0x00, 0x30,
        0x00, 0x00, 0x07, 0xEB, 0x81, 0x07, 0xEB, 0x81, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x38, 0x00, 0x00, 0x34, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    }
};