
#ifndef __TEST_HAL_I2C_24C256__
#define __TEST_HAL_I2C_24C256__

#ifdef __cplusplus
extern "C"
{
#endif


#define TAR_24C256_ADDR (0x51)

void commandhal_Test24C256(char* i2c_num_str,char* i2c_value);


#ifdef __cplusplus
}
#endif

#endif