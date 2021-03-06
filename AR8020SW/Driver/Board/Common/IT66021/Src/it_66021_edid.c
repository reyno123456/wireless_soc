#if 0
#include <stddef.h>
#include <stdint.h>

unsigned char hdmi_edid_table[257][3] =
{
    {0xA8, 0x00, 0x00},
    {0xA8, 0x01, 0xFF},
    {0xA8, 0x02, 0xFF},
    {0xA8, 0x03, 0xFF},
    {0xA8, 0x04, 0xFF},
    {0xA8, 0x05, 0xFF},
    {0xA8, 0x06, 0xFF},
    {0xA8, 0x07, 0x00},
    {0xA8, 0x08, 0x26},
    {0xA8, 0x09, 0x85},
    {0xA8, 0x0A, 0x02},
    {0xA8, 0x0B, 0x66},
    {0xA8, 0x0C, 0x01},
    {0xA8, 0x0D, 0x01},
    {0xA8, 0x0E, 0x01},
    {0xA8, 0x0F, 0x01},
    {0xA8, 0x10, 0x21},
    {0xA8, 0x11, 0x17},
    {0xA8, 0x12, 0x01},
    {0xA8, 0x13, 0x03},
    {0xA8, 0x14, 0x80},
    {0xA8, 0x15, 0x55},
    {0xA8, 0x16, 0x30},
    {0xA8, 0x17, 0x78},
    {0xA8, 0x18, 0x2A},
    {0xA8, 0x19, 0x63},
    {0xA8, 0x1A, 0xBD},
    {0xA8, 0x1B, 0xA1},
    {0xA8, 0x1C, 0x54},
    {0xA8, 0x1D, 0x52},
    {0xA8, 0x1E, 0x9E},
    {0xA8, 0x1F, 0x26},
    {0xA8, 0x20, 0x0C},
    {0xA8, 0x21, 0x47},
    {0xA8, 0x22, 0x4A},
    {0xA8, 0x23, 0x20},
    {0xA8, 0x24, 0x08},
    {0xA8, 0x25, 0x00},
    {0xA8, 0x26, 0x81},
    {0xA8, 0x27, 0x00},
    {0xA8, 0x28, 0xD1},
    {0xA8, 0x29, 0xC0},
    {0xA8, 0x2A, 0x01},
    {0xA8, 0x2B, 0x01},
    {0xA8, 0x2C, 0x01},
    {0xA8, 0x2D, 0x01},
    {0xA8, 0x2E, 0x01},
    {0xA8, 0x2F, 0x01},
    {0xA8, 0x30, 0x01},
    {0xA8, 0x31, 0x01},
    {0xA8, 0x32, 0x01},
    {0xA8, 0x33, 0x01},
    {0xA8, 0x34, 0x01},
    {0xA8, 0x35, 0x01},
    {0xA8, 0x36, 0x04},
    {0xA8, 0x37, 0x74},
    {0xA8, 0x38, 0x00},
    {0xA8, 0x39, 0x30},
    {0xA8, 0x3A, 0xF2},
    {0xA8, 0x3B, 0x70},
    {0xA8, 0x3C, 0x5A},
    {0xA8, 0x3D, 0x80},
    {0xA8, 0x3E, 0xB0},
    {0xA8, 0x3F, 0x58},
    {0xA8, 0x40, 0x8A},
    {0xA8, 0x41, 0x00},
    {0xA8, 0x42, 0xA2},
    {0xA8, 0x43, 0x0B},
    {0xA8, 0x44, 0x32},
    {0xA8, 0x45, 0x00},
    {0xA8, 0x46, 0x00},
    {0xA8, 0x47, 0x1E},
    {0xA8, 0x48, 0x01},
    {0xA8, 0x49, 0x1D},
    {0xA8, 0x4A, 0x00},
    {0xA8, 0x4B, 0x72},
    {0xA8, 0x4C, 0x51},
    {0xA8, 0x4D, 0xD0},
    {0xA8, 0x4E, 0x1E},
    {0xA8, 0x4F, 0x20},
    {0xA8, 0x50, 0x6E},
    {0xA8, 0x51, 0x28},
    {0xA8, 0x52, 0x55},
    {0xA8, 0x53, 0x00},
    {0xA8, 0x54, 0xC4},
    {0xA8, 0x55, 0x8E},
    {0xA8, 0x56, 0x21},
    {0xA8, 0x57, 0x00},
    {0xA8, 0x58, 0x00},
    {0xA8, 0x59, 0x1E},
    {0xA8, 0x5A, 0x00},
    {0xA8, 0x5B, 0x00},
    {0xA8, 0x5C, 0x00},
    {0xA8, 0x5D, 0xFD},
    {0xA8, 0x5E, 0x00},
    {0xA8, 0x5F, 0x18},
    {0xA8, 0x60, 0x4C},
    {0xA8, 0x61, 0x1E},
    {0xA8, 0x62, 0x53},
    {0xA8, 0x63, 0x1E},
    {0xA8, 0x64, 0x00},
    {0xA8, 0x65, 0x0A},
    {0xA8, 0x66, 0x20},
    {0xA8, 0x67, 0x20},
    {0xA8, 0x68, 0x20},
    {0xA8, 0x69, 0x20},
    {0xA8, 0x6A, 0x20},
    {0xA8, 0x6B, 0x20},
    {0xA8, 0x6C, 0x00},
    {0xA8, 0x6D, 0x00},
    {0xA8, 0x6E, 0x00},
    {0xA8, 0x6F, 0xFC},
    {0xA8, 0x70, 0x00},
    {0xA8, 0x71, 0x49},
    {0xA8, 0x72, 0x54},
    {0xA8, 0x73, 0x45},
    {0xA8, 0x74, 0x36},
    {0xA8, 0x75, 0x38},
    {0xA8, 0x76, 0x30},
    {0xA8, 0x77, 0x32},
    {0xA8, 0x78, 0x0A},
    {0xA8, 0x79, 0x20},
    {0xA8, 0x7A, 0x20},
    {0xA8, 0x7B, 0x20},
    {0xA8, 0x7C, 0x20},
    {0xA8, 0x7D, 0x20},
    {0xA8, 0x7E, 0x01},
    {0xA8, 0x7F, 0x8B},
    {0xA8, 0x80, 0x02},
    {0xA8, 0x81, 0x03},
    {0xA8, 0x82, 0x25},
    {0xA8, 0x83, 0xF1},
    {0xA8, 0x84, 0x43},
    {0xA8, 0x85, 0x84},
    {0xA8, 0x86, 0x10},
    {0xA8, 0x87, 0x03},
    {0xA8, 0x88, 0x23},
    {0xA8, 0x89, 0x09},
    {0xA8, 0x8A, 0x07},
    {0xA8, 0x8B, 0x07},
    {0xA8, 0x8C, 0x83},
    {0xA8, 0x8D, 0x01},
    {0xA8, 0x8E, 0x00},
    {0xA8, 0x8F, 0x00},
    {0xA8, 0x90, 0xE2},
    {0xA8, 0x91, 0x00},
    {0xA8, 0x92, 0x0F},
    {0xA8, 0x93, 0xE3},
    {0xA8, 0x94, 0x05},
    {0xA8, 0x95, 0x03},
    {0xA8, 0x96, 0x01},
    {0xA8, 0x97, 0x6D},
    {0xA8, 0x98, 0x03},
    {0xA8, 0x99, 0x0C},
    {0xA8, 0x9A, 0x00},
    {0xA8, 0x9B, 0x10},
    {0xA8, 0x9C, 0x00},
    {0xA8, 0x9D, 0x38},
    {0xA8, 0x9E, 0x3C},
    {0xA8, 0x9F, 0x20},
    {0xA8, 0xA0, 0x00},
    {0xA8, 0xA1, 0x60},
    {0xA8, 0xA2, 0x03},
    {0xA8, 0xA3, 0x02},
    {0xA8, 0xA4, 0x01},
    {0xA8, 0xA5, 0x01},
    {0xA8, 0xA6, 0x1D},
    {0xA8, 0xA7, 0x00},
    {0xA8, 0xA8, 0x72},
    {0xA8, 0xA9, 0x51},
    {0xA8, 0xAA, 0xD0},
    {0xA8, 0xAB, 0x1E},
    {0xA8, 0xAC, 0x20},
    {0xA8, 0xAD, 0x6E},
    {0xA8, 0xAE, 0x28},
    {0xA8, 0xAF, 0x55},
    {0xA8, 0xB0, 0x00},
    {0xA8, 0xB1, 0xA0},
    {0xA8, 0xB2, 0x5A},
    {0xA8, 0xB3, 0x00},
    {0xA8, 0xB4, 0x00},
    {0xA8, 0xB5, 0x00},
    {0xA8, 0xB6, 0x1E},
    {0xA8, 0xB7, 0x8C},
    {0xA8, 0xB8, 0x0A},
    {0xA8, 0xB9, 0xD0},
    {0xA8, 0xBA, 0x8A},
    {0xA8, 0xBB, 0x20},
    {0xA8, 0xBC, 0xE0},
    {0xA8, 0xBD, 0x2D},
    {0xA8, 0xBE, 0x10},
    {0xA8, 0xBF, 0x10},
    {0xA8, 0xC0, 0x3E},
    {0xA8, 0xC1, 0x96},
    {0xA8, 0xC2, 0x00},
    {0xA8, 0xC3, 0xA0},
    {0xA8, 0xC4, 0x5A},
    {0xA8, 0xC5, 0x00},
    {0xA8, 0xC6, 0x00},
    {0xA8, 0xC7, 0x00},
    {0xA8, 0xC8, 0x18},
    {0xA8, 0xC9, 0xF3},
    {0xA8, 0xCA, 0x39},
    {0xA8, 0xCB, 0x80},
    {0xA8, 0xCC, 0x18},
    {0xA8, 0xCD, 0x71},
    {0xA8, 0xCE, 0x38},
    {0xA8, 0xCF, 0x2D},
    {0xA8, 0xD0, 0x40},
    {0xA8, 0xD1, 0x58},
    {0xA8, 0xD2, 0x2C},
    {0xA8, 0xD3, 0x45},
    {0xA8, 0xD4, 0x00},
    {0xA8, 0xD5, 0xE0},
    {0xA8, 0xD6, 0x0E},
    {0xA8, 0xD7, 0x11},
    {0xA8, 0xD8, 0x00},
    {0xA8, 0xD9, 0x00},
    {0xA8, 0xDA, 0x1E},
    {0xA8, 0xDB, 0x00},
    {0xA8, 0xDC, 0x00},
    {0xA8, 0xDD, 0x00},
    {0xA8, 0xDE, 0x00},
    {0xA8, 0xDF, 0x00},
    {0xA8, 0xE0, 0x00},
    {0xA8, 0xE1, 0x00},
    {0xA8, 0xE2, 0x00},
    {0xA8, 0xE3, 0x00},
    {0xA8, 0xE4, 0x00},
    {0xA8, 0xE5, 0x00},
    {0xA8, 0xE6, 0x00},
    {0xA8, 0xE7, 0x00},
    {0xA8, 0xE8, 0x00},
    {0xA8, 0xE9, 0x00},
    {0xA8, 0xEA, 0x00},
    {0xA8, 0xEB, 0x00},
    {0xA8, 0xEC, 0x00},
    {0xA8, 0xED, 0x00},
    {0xA8, 0xEE, 0x00},
    {0xA8, 0xEF, 0x00},
    {0xA8, 0xF0, 0x00},
    {0xA8, 0xF1, 0x00},
    {0xA8, 0xF2, 0x00},
    {0xA8, 0xF3, 0x00},
    {0xA8, 0xF4, 0x00},
    {0xA8, 0xF5, 0x00},
    {0xA8, 0xF6, 0x00},
    {0xA8, 0xF7, 0x00},
    {0xA8, 0xF8, 0x00},
    {0xA8, 0xF9, 0x00},
    {0xA8, 0xFA, 0x00},
    {0xA8, 0xFB, 0x00},
    {0xA8, 0xFC, 0x00},
    {0xA8, 0xFD, 0x00},
    {0xA8, 0xFE, 0x00},
    {0xA8, 0xFF, 0x15},
    {0xFF, 0xFF, 0xFF}
};
#endif
#if 1
#include <stddef.h>
#include <stdint.h>

unsigned char hdmi_edid_table[257][3] =
{
    {0xA8, 0x00, 0x00},
    {0xA8, 0x01, 0xFF},
    {0xA8, 0x02, 0xFF},
    {0xA8, 0x03, 0xFF},
    {0xA8, 0x04, 0xFF},
    {0xA8, 0x05, 0xFF},
    {0xA8, 0x06, 0xFF},
    {0xA8, 0x07, 0x00},
    {0xA8, 0x08, 0x26},
    {0xA8, 0x09, 0x85},
    {0xA8, 0x0A, 0x02},
    {0xA8, 0x0B, 0x68},
    {0xA8, 0x0C, 0x01},
    {0xA8, 0x0D, 0x68},
    {0xA8, 0x0E, 0x00},
    {0xA8, 0x0F, 0x00},
    {0xA8, 0x10, 0x00},
    {0xA8, 0x11, 0x17},
    {0xA8, 0x12, 0x01},
    {0xA8, 0x13, 0x03},
    {0xA8, 0x14, 0x80},
    {0xA8, 0x15, 0x73},
    {0xA8, 0x16, 0x41},
    {0xA8, 0x17, 0x78},
    {0xA8, 0x18, 0x2A},
    {0xA8, 0x19, 0x7C},
    {0xA8, 0x1A, 0x11},
    {0xA8, 0x1B, 0x9E},
    {0xA8, 0x1C, 0x59},
    {0xA8, 0x1D, 0x47},
    {0xA8, 0x1E, 0x9B},
    {0xA8, 0x1F, 0x27},
    {0xA8, 0x20, 0x10},
    {0xA8, 0x21, 0x50},
    {0xA8, 0x22, 0x54},
    {0xA8, 0x23, 0x00},
    {0xA8, 0x24, 0x00},
    {0xA8, 0x25, 0x00},
    {0xA8, 0x26, 0x01},
    {0xA8, 0x27, 0x01},
    {0xA8, 0x28, 0x01},
    {0xA8, 0x29, 0x01},
    {0xA8, 0x2A, 0x01},
    {0xA8, 0x2B, 0x01},
    {0xA8, 0x2C, 0x01},
    {0xA8, 0x2D, 0x01},
    {0xA8, 0x2E, 0x01},
    {0xA8, 0x2F, 0x01},
    {0xA8, 0x30, 0x01},
    {0xA8, 0x31, 0x01},
    {0xA8, 0x32, 0x01},
    {0xA8, 0x33, 0x01},
    {0xA8, 0x34, 0x01},
    {0xA8, 0x35, 0x01},
    {0xA8, 0x36, 0x02},
    {0xA8, 0x37, 0x3A},
    {0xA8, 0x38, 0x80},
    {0xA8, 0x39, 0x18},
    {0xA8, 0x3A, 0x71},
    {0xA8, 0x3B, 0x38},
    {0xA8, 0x3C, 0x2D},
    {0xA8, 0x3D, 0x40},
    {0xA8, 0x3E, 0x58},
    {0xA8, 0x3F, 0x2C},
    {0xA8, 0x40, 0x45},
    {0xA8, 0x41, 0x00},
    {0xA8, 0x42, 0x10},
    {0xA8, 0x43, 0x09},
    {0xA8, 0x44, 0x00},
    {0xA8, 0x45, 0x00},
    {0xA8, 0x46, 0x00},
    {0xA8, 0x47, 0x1E},
    {0xA8, 0x48, 0x8C},
    {0xA8, 0x49, 0x0A},
    {0xA8, 0x4A, 0xD0},
    {0xA8, 0x4B, 0x8A},
    {0xA8, 0x4C, 0x20},
    {0xA8, 0x4D, 0xE0},
    {0xA8, 0x4E, 0x2D},
    {0xA8, 0x4F, 0x10},
    {0xA8, 0x50, 0x10},
    {0xA8, 0x51, 0x3E},
    {0xA8, 0x52, 0x96},
    {0xA8, 0x53, 0x00},
    {0xA8, 0x54, 0x04},
    {0xA8, 0x55, 0x03},
    {0xA8, 0x56, 0x00},
    {0xA8, 0x57, 0x00},
    {0xA8, 0x58, 0x00},
    {0xA8, 0x59, 0x18},
    {0xA8, 0x5A, 0x00},
    {0xA8, 0x5B, 0x00},
    {0xA8, 0x5C, 0x00},
    {0xA8, 0x5D, 0xFC},
    {0xA8, 0x5E, 0x00},
    {0xA8, 0x5F, 0x49},
    {0xA8, 0x60, 0x54},
    {0xA8, 0x61, 0x45},
    {0xA8, 0x62, 0x36},
    {0xA8, 0x63, 0x38},
    {0xA8, 0x64, 0x30},
    {0xA8, 0x65, 0x32},
    {0xA8, 0x66, 0x0A},
    {0xA8, 0x67, 0x20},
    {0xA8, 0x68, 0x20},
    {0xA8, 0x69, 0x20},
    {0xA8, 0x6A, 0x20},
    {0xA8, 0x6B, 0x20},
    {0xA8, 0x6C, 0x00},
    {0xA8, 0x6D, 0x00},
    {0xA8, 0x6E, 0x00},
    {0xA8, 0x6F, 0xFD},
    {0xA8, 0x70, 0x00},
    {0xA8, 0x71, 0x30},
    {0xA8, 0x72, 0x7A},
    {0xA8, 0x73, 0x0F},
    {0xA8, 0x74, 0x50},
    {0xA8, 0x75, 0x10},
    {0xA8, 0x76, 0x00},
    {0xA8, 0x77, 0x0A},
    {0xA8, 0x78, 0x20},
    {0xA8, 0x79, 0x20},
    {0xA8, 0x7A, 0x20},
    {0xA8, 0x7B, 0x20},
    {0xA8, 0x7C, 0x20},
    {0xA8, 0x7D, 0x20},
    {0xA8, 0x7E, 0x01},
    {0xA8, 0x7F, 0xF3},
    {0xA8, 0x80, 0x02},
    {0xA8, 0x81, 0x03},
    {0xA8, 0x82, 0x1B},
    {0xA8, 0x83, 0x72},
    {0xA8, 0x84, 0x48},
    {0xA8, 0x85, 0x90},
    {0xA8, 0x86, 0x04},
    {0xA8, 0x87, 0x13},
    {0xA8, 0x88, 0x01},
    {0xA8, 0x89, 0x02},
    {0xA8, 0x8A, 0x03},
    {0xA8, 0x8B, 0x06},
    {0xA8, 0x8C, 0x87},
    {0xA8, 0x8D, 0x23},
    {0xA8, 0x8E, 0x09},
    {0xA8, 0x8F, 0x07},
    {0xA8, 0x90, 0x07},
    {0xA8, 0x91, 0x83},
    {0xA8, 0x92, 0x01},
    {0xA8, 0x93, 0x00},
    {0xA8, 0x94, 0x00},
    {0xA8, 0x95, 0x65},
    {0xA8, 0x96, 0x03},
    {0xA8, 0x97, 0x0C},
    {0xA8, 0x98, 0x00},
    {0xA8, 0x99, 0x10},
    {0xA8, 0x9A, 0x00},
    {0xA8, 0x9B, 0x01},
    {0xA8, 0x9C, 0x1D},
    {0xA8, 0x9D, 0x00},
    {0xA8, 0x9E, 0x72},
    {0xA8, 0x9F, 0x51},
    {0xA8, 0xA0, 0xD0},
    {0xA8, 0xA1, 0x1E},
    {0xA8, 0xA2, 0x20},
    {0xA8, 0xA3, 0x6E},
    {0xA8, 0xA4, 0x28},
    {0xA8, 0xA5, 0x55},
    {0xA8, 0xA6, 0x00},
    {0xA8, 0xA7, 0x10},
    {0xA8, 0xA8, 0x09},
    {0xA8, 0xA9, 0x00},
    {0xA8, 0xAA, 0x00},
    {0xA8, 0xAB, 0x00},
    {0xA8, 0xAC, 0x1E},
    {0xA8, 0xAD, 0xD6},
    {0xA8, 0xAE, 0x09},
    {0xA8, 0xAF, 0x80},
    {0xA8, 0xB0, 0xA0},
    {0xA8, 0xB1, 0x20},
    {0xA8, 0xB2, 0xE0},
    {0xA8, 0xB3, 0x2D},
    {0xA8, 0xB4, 0x10},
    {0xA8, 0xB5, 0x10},
    {0xA8, 0xB6, 0x60},
    {0xA8, 0xB7, 0xA2},
    {0xA8, 0xB8, 0x00},
    {0xA8, 0xB9, 0x04},
    {0xA8, 0xBA, 0x03},
    {0xA8, 0xBB, 0x00},
    {0xA8, 0xBC, 0x00},
    {0xA8, 0xBD, 0x00},
    {0xA8, 0xBE, 0x18},
    {0xA8, 0xBF, 0x8C},
    {0xA8, 0xC0, 0x0A},
    {0xA8, 0xC1, 0xD0},
    {0xA8, 0xC2, 0x8A},
    {0xA8, 0xC3, 0x20},
    {0xA8, 0xC4, 0xE0},
    {0xA8, 0xC5, 0x2D},
    {0xA8, 0xC6, 0x10},
    {0xA8, 0xC7, 0x10},
    {0xA8, 0xC8, 0x3E},
    {0xA8, 0xC9, 0x96},
    {0xA8, 0xCA, 0x00},
    {0xA8, 0xCB, 0x10},
    {0xA8, 0xCC, 0x09},
    {0xA8, 0xCD, 0x00},
    {0xA8, 0xCE, 0x00},
    {0xA8, 0xCF, 0x00},
    {0xA8, 0xD0, 0x18},
    {0xA8, 0xD1, 0x8C},
    {0xA8, 0xD2, 0x0A},
    {0xA8, 0xD3, 0xA0},
    {0xA8, 0xD4, 0x14},
    {0xA8, 0xD5, 0x51},
    {0xA8, 0xD6, 0xF0},
    {0xA8, 0xD7, 0x16},
    {0xA8, 0xD8, 0x00},
    {0xA8, 0xD9, 0x26},
    {0xA8, 0xDA, 0x7C},
    {0xA8, 0xDB, 0x43},
    {0xA8, 0xDC, 0x00},
    {0xA8, 0xDD, 0xD0},
    {0xA8, 0xDE, 0xE0},
    {0xA8, 0xDF, 0x21},
    {0xA8, 0xE0, 0x00},
    {0xA8, 0xE1, 0x00},
    {0xA8, 0xE2, 0x98},
    {0xA8, 0xE3, 0x00},
    {0xA8, 0xE4, 0x00},
    {0xA8, 0xE5, 0x00},
    {0xA8, 0xE6, 0x00},
    {0xA8, 0xE7, 0x00},
    {0xA8, 0xE8, 0x00},
    {0xA8, 0xE9, 0x00},
    {0xA8, 0xEA, 0x00},
    {0xA8, 0xEB, 0x00},
    {0xA8, 0xEC, 0x00},
    {0xA8, 0xED, 0x00},
    {0xA8, 0xEE, 0x00},
    {0xA8, 0xEF, 0x00},
    {0xA8, 0xF0, 0x00},
    {0xA8, 0xF1, 0x00},
    {0xA8, 0xF2, 0x00},
    {0xA8, 0xF3, 0x00},
    {0xA8, 0xF4, 0x00},
    {0xA8, 0xF5, 0x00},
    {0xA8, 0xF6, 0x00},
    {0xA8, 0xF7, 0x00},
    {0xA8, 0xF8, 0x00},
    {0xA8, 0xF9, 0x00},
    {0xA8, 0xFA, 0x00},
    {0xA8, 0xFB, 0x00},
    {0xA8, 0xFC, 0x00},
    {0xA8, 0xFD, 0x00},
    {0xA8, 0xFE, 0x00},
    {0xA8, 0xFF, 0x7B},
    {0xFF, 0xFF, 0xFF}
};
#endif