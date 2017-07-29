#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debuglog.h"
#include "hal_ret_type.h"
#include "hal_i2c.h"
#include "test_hal_i2c.h"

#define I2C_TIMEOUT_VALUE    (2)

static uint16_t u16_i2cAddr[6];

void command_TestHalI2cInit(uint8_t *ch, uint8_t *i2c_addr, uint8_t *speed)
{
    uint32_t u32_ch = strtoul(ch, NULL, 0);
    uint32_t u32_addr = (uint16_t)strtoul(i2c_addr, NULL, 0);
    uint32_t u32_speed = strtoul(speed, NULL, 0);

    u16_i2cAddr[u32_ch] = (uint16_t)u32_addr;

    HAL_I2C_MasterInit(u32_ch,  u16_i2cAddr[u32_ch], (ENUM_HAL_I2C_SPEED)u32_speed);

    dlog_info("e_i2cComponent:%d i2cAddr:0x%x speed:%d",
               u32_ch,  u16_i2cAddr[u32_ch], u32_speed);
}

void command_TestHalI2cWrite(uint8_t *ch, uint8_t *subAddr, uint8_t *subAddrLen, uint8_t *data, uint8_t *dataLen)
{
    uint8_t u8_addr_data[8];
    uint8_t *pu8_addr;
    uint8_t i = 0;

    uint32_t u32_ch = strtoul(ch, NULL, 0);
    uint32_t u32_addr = strtoul(subAddr, NULL, 0);
    uint32_t u32_addrLen = strtoul(subAddrLen, NULL, 0);
    uint32_t u32_data = strtoul(data, NULL, 0);
    uint32_t u32_dataLen = strtoul(dataLen, NULL, 0);

    u32_addrLen = (u32_addrLen > 4)? 4 : u32_addrLen;
    u32_dataLen = (u32_dataLen > 4)? 4 : u32_dataLen;

    while(i < u32_addrLen)
    {
        u8_addr_data[i] = (u32_addr >> ((u32_addrLen - i -1) * 8)) & 0xFF;
        i++;
    }

    i = 0;
    pu8_addr = &u8_addr_data[u32_addrLen];
    while(i < u32_dataLen)
    {
        pu8_addr[i] = (u32_data >> ((u32_dataLen - i -1) * 8)) & 0xFF;
        i++;
    }
    
    HAL_I2C_MasterWriteData(u32_ch, 
                            u16_i2cAddr[u32_ch],
                            u8_addr_data,
                            u32_addrLen + u32_dataLen,
                            I2C_TIMEOUT_VALUE);

    dlog_info("e_i2cComponent:%d i2cAddr:0x%x",u32_ch,  u16_i2cAddr[u32_ch]);

    for(i = 0; i < (u32_addrLen + u32_dataLen); i++)
    {
        dlog_info("addr_data[%d]:0x%x", i, u8_addr_data[i]);
    }
}

void command_TestHalI2cRead(uint8_t *ch, uint8_t *subAddr, uint8_t *subAddrLen, uint8_t *dataLen)
{
    uint8_t u8_addr_data[8];
    uint8_t u8_rxdata[8];
    uint8_t *pu8_addr;
    uint8_t i = 0;

    uint32_t u32_ch = strtoul(ch, NULL, 0);
    uint32_t u32_addr = strtoul(subAddr, NULL, 0);
    uint32_t u32_addrLen = strtoul(subAddrLen, NULL, 0);
    uint32_t u32_dataLen = strtoul(dataLen, NULL, 0);

    u32_addrLen = (u32_addrLen > 4)? 4 : u32_addrLen;
    u32_dataLen = (u32_dataLen > 4)? 4 : u32_dataLen;

    while(i < u32_addrLen)
    {
        u8_addr_data[i] = (u32_addr >> ((u32_addrLen - i -1) * 8)) & 0xFF;
        i++;
    }

    HAL_I2C_MasterReadData(u32_ch, 
                           u16_i2cAddr[u32_ch],
                           u8_addr_data,
                           u32_addrLen,
                           u8_rxdata,
                           u32_dataLen,
                           I2C_TIMEOUT_VALUE);


    dlog_info("e_i2cComponent:%d i2cAddr:0x%x",u32_ch,  u16_i2cAddr[u32_ch]);

    for(i = 0; i < u32_addrLen; i++)
    {
        dlog_info("addr[%d]:0x%x", i, u8_addr_data[i]);
    }

    for(i = 0; i < u32_dataLen; i++)
    {
        dlog_info("rxdata[%d]:0x%x", i, u8_rxdata[i]);
    }
} 


