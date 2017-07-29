#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "hal_ret_type.h"
#include "hal_i2c.h"
#include "test_hal_i2c_24c256.h"
#include "debuglog.h"
#include "hal.h"

#define I2C_24C256_TIMEOUT_VALUE    (10)
#define TEST_24C256_BUFFSIZE 64

static void testhal_24c256(unsigned char i2c_num,unsigned char value);

void testhal_24c256(unsigned char i2c_num,unsigned char value)
{
    HAL_I2C_MasterInit(i2c_num, TAR_24C256_ADDR, HAL_I2C_FAST_SPEED);
    dlog_info("i2c_num %d I2C_Initial finished!\n", i2c_num);
    unsigned char i=0;
    unsigned char data_src1[TEST_24C256_BUFFSIZE+2] = {0x00,0x00};
    unsigned char data_chk[TEST_24C256_BUFFSIZE];
    unsigned short rd_start_addr = 0;
    
    for (i = 0; i< TEST_24C256_BUFFSIZE; i++)
    {
        data_src1[i+2] = value + i;
    }

    HAL_I2C_MasterWriteData(i2c_num, TAR_24C256_ADDR, data_src1, 66, I2C_24C256_TIMEOUT_VALUE);
    dlog_info("I2C_Master_Write_Data finished!\n");
    HAL_Delay(200);
    memset((void *)data_chk, 0, sizeof(data_chk));
    HAL_I2C_MasterReadData(i2c_num, TAR_24C256_ADDR, data_src1, 2, data_chk, TEST_24C256_BUFFSIZE, I2C_24C256_TIMEOUT_VALUE);

    for(i = 0; i < TEST_24C256_BUFFSIZE; i++)
    {

        dlog_info("%d", data_chk[i]);
        if (data_src1[i+2] != data_chk[i])
        {
            dlog_info("\ndata_src = %d, data_chk = %d\n", data_src1[i+2], data_chk[i]);
            dlog_output(100);
        }
    }

    dlog_info("I2C_Master_Read_Data finished!\n");

}

void commandhal_Test24C256(char* i2c_num_str,char* i2c_value)
{
    testhal_24c256(strtoul(i2c_num_str, NULL, 0),strtoul(i2c_value, NULL, 0));
}