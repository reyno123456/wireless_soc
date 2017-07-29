#include <stdlib.h>
#include <string.h>
#include "debuglog.h"
#include "i2c.h"
#include "adv_7611.h"
#include "test_hal_i2c_24c256.h"

#if 1
#define TAR_ADV7611_ADDR      (0x98 >> 1)

#define ADV_7611_I2C_COMPONENT_NUM I2C_Component_2
#define ADV_7611_I2C_MAX_DELAY      (2)

#define RX_I2C_IO_MAP_ADDR              (0x98 >> 1)
#define RX_I2C_CP_MAP_ADDR              (0x44 >> 1)
#define RX_I2C_VDP_MAP_ADDR             (0x48 >> 1)
#define RX_I2C_AFE_DPLL_MAP_ADDR        (0x4C >> 1)
#define RX_I2C_REPEATER_MAP_ADDR        (0x64 >> 1)
#define RX_I2C_HDMI_MAP_ADDR            (0x68 >> 1)
#define RX_I2C_EDID_MAP_ADDR            (0x6C >> 1)
#define RX_I2C_ESDP_MAP_ADDR            (0x70 >> 1)
#define RX_I2C_DPP_MAP_ADDR             (0x78 >> 1)
#define RX_I2C_AVLINK_MAP_ADDR          (0x84 >> 1)
#define RX_I2C_CEC_MAP_ADDR             (0x80 >> 1)
#define RX_I2C_INFOFRAME_MAP_ADDR       (0x7C >> 1)
#define RX_I2C_TEST_MAP1_ADDR           (0x60 >> 1)
#define RX_I2C_SDP_MAP_ADDR             (0x90 >> 1)
#define RX_I2C_SDP_IO_MAP_ADDR          (0x94 >> 1)

static void ADV_7611_WriteByte(unsigned char slv_addr, unsigned char sub_addr, unsigned char val)
{
    unsigned char data[2] = {sub_addr, val};
    I2C_Master_WriteData(ADV_7611_I2C_COMPONENT_NUM, slv_addr >> 1, data, 2);
    I2C_Master_WaitTillIdle(ADV_7611_I2C_COMPONENT_NUM, ADV_7611_I2C_MAX_DELAY);
}

static unsigned char ADV_7611_ReadByte(unsigned char slv_addr, unsigned char sub_addr)
{
    unsigned char sub_addr_tmp = sub_addr;
    unsigned char val = 0;
    I2C_Master_ReadData(ADV_7611_I2C_COMPONENT_NUM, slv_addr >> 1, &sub_addr_tmp, 1, &val, 1);
    I2C_Master_WaitTillIdle(ADV_7611_I2C_COMPONENT_NUM, ADV_7611_I2C_MAX_DELAY);
    return val;
}
#endif

void command_readADV7611(char *slvAddr, char *regAddr)
{
    unsigned char slvAddrTmp = strtoul(slvAddr, NULL, 0);
    unsigned char regAddrTmp = strtoul(regAddr, NULL, 0);
    unsigned char val = ADV_7611_ReadByte(slvAddrTmp, regAddrTmp);
    dlog_info("Read: 0x%x, 0x%x. Val: 0x%x", slvAddrTmp, regAddrTmp, val);
}

void command_writeADV7611(char *slvAddr, char *regAddr, char *regVal)
{
    unsigned char slvAddrTmp = strtoul(slvAddr, NULL, 0);
    unsigned char regAddrTmp = strtoul(regAddr, NULL, 0);
    unsigned char regValTmp  = strtoul(regVal, NULL, 0);
    ADV_7611_WriteByte(slvAddrTmp, regAddrTmp, regValTmp);
    dlog_info("Wrte: 0x%x, 0x%x, 0x%x", slvAddrTmp, regAddrTmp, regValTmp);
}

void command_readADV7611VideoFormat(char *index_str, uint16_t* widthPtr, uint16_t* hightPtr, uint8_t* framteratePtr)
{
    unsigned char index = strtoul(index_str, NULL, 0);
    ADV_7611_GetVideoFormat(index, widthPtr, hightPtr, framteratePtr);
}

void command_initADV7611(char *index_str)
{
    unsigned char index = strtoul(index_str, NULL, 0);
    ADV_7611_Initial(index);
    dlog_info("ADV7611 Initial finished!\n");
}

void command_dumpADV7611Settings(char *index_str)
{
    unsigned char index = strtoul(index_str, NULL, 0);
    ADV_7611_DumpOutEdidData(index);
    ADV_7611_DumpOutDefaultSettings(index);
}



